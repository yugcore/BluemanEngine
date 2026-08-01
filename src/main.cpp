#include <iostream>
#include <vector>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_dx12.h>

#include "theme/Theme.h"
#include "app/Application.h"
#include "core/CommandStack.h"
#include "panels/Chrome/CustomTitleBar.h"
#include "render/ViewportRenderer.h"

using Microsoft::WRL::ComPtr;

struct FrameContext {
    ComPtr<ID3D12CommandAllocator> commandAllocator;
    UINT64 fenceValue = 0;
};

static const int NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_FrameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_FrameIndex = 0;

static const int NUM_BACK_BUFFERS = 3;
static ComPtr<ID3D12Device> g_pd3dDevice;
static ComPtr<ID3D12CommandQueue> g_pd3dCommandQueue;
static ComPtr<IDXGISwapChain3> g_pSwapChain;
static ComPtr<ID3D12DescriptorHeap> g_pd3dRtvDescHeap;
static ComPtr<ID3D12DescriptorHeap> g_pd3dSrvDescHeap;
static ComPtr<ID3D12GraphicsCommandList> g_pd3dCommandList;
static ComPtr<ID3D12Resource> g_mainRenderTargetResource[NUM_BACK_BUFFERS];
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS];

static ComPtr<ID3D12Fence> g_pFence;
static HANDLE g_hFenceEvent = NULL;
static UINT64 g_fenceLastSignaledValue = 0;

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "[GLFW Error] (" << error << "): " << description << std::endl;
}

static bool CreateDeviceD3D(HWND hWnd);
static void CleanupDeviceD3D();
static void CreateRenderTarget();
static void CleanupRenderTarget();
static void WaitForLastSubmittedFrame();
static FrameContext* WaitForNextFrameResources();

static UINT g_srvHeapNextFreeIndex = 0;

static void SrvDescriptorAlloc(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
    UINT handleIncrement = info->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    
    cpuHandle.ptr += (SIZE_T)g_srvHeapNextFreeIndex * handleIncrement;
    gpuHandle.ptr += (SIZE_T)g_srvHeapNextFreeIndex * handleIncrement;
    
    g_srvHeapNextFreeIndex++;
    
    *out_cpu_handle = cpuHandle;
    *out_gpu_handle = gpuHandle;
}

static void SrvDescriptorFree(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
    (void)info; (void)cpu_handle; (void)gpu_handle;
}

static UINT g_SwapChainWidth = 0;
static UINT g_SwapChainHeight = 0;

static void FlushCommandQueue() {
    if (!g_pd3dCommandQueue || !g_pFence || !g_hFenceEvent) return;
    g_fenceLastSignaledValue++;
    g_pd3dCommandQueue->Signal(g_pFence.Get(), g_fenceLastSignaledValue);
    if (g_pFence->GetCompletedValue() < g_fenceLastSignaledValue) {
        g_pFence->SetEventOnCompletion(g_fenceLastSignaledValue, g_hFenceEvent);
        WaitForSingleObject(g_hFenceEvent, INFINITE);
    }
}

void WaitForGPU() {
    FlushCommandQueue();
}

static GLFWwindow* g_Window = nullptr;
static WNDPROC g_PrevWndProc = NULL;
static bool s_InFrameRender = false;

static void RenderFrame() {
    if (s_InFrameRender || !g_Window || !g_pSwapChain) return;
    s_InFrameRender = true;

    int display_w = 0, display_h = 0;
    glfwGetFramebufferSize(g_Window, &display_w, &display_h);

    if (display_w == 0 || display_h == 0 || glfwGetWindowAttrib(g_Window, GLFW_ICONIFIED)) {
        s_InFrameRender = false;
        return;
    }

    // Handle SwapChain Resizing
    if (g_SwapChainWidth != (UINT)display_w || g_SwapChainHeight != (UINT)display_h) {
        FlushCommandQueue();
        CleanupRenderTarget();
        HRESULT hr = g_pSwapChain->ResizeBuffers(
            0,
            (UINT)display_w,
            (UINT)display_h,
            DXGI_FORMAT_UNKNOWN,
            DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT
        );
        g_SwapChainWidth = (UINT)display_w;
        g_SwapChainHeight = (UINT)display_h;
        if (SUCCEEDED(hr)) {
            CreateRenderTarget();
        } else {
            std::cerr << "[DX12] ResizeBuffers failed (hr = 0x" << std::hex << hr << ")" << std::endl;
        }
    }

    // 1. BeginFrame & Wait for Next Frame Resources
    FrameContext* frameCtx = WaitForNextFrameResources();
    g_FrameIndex = g_pSwapChain->GetCurrentBackBufferIndex();

    // 2. Reset Command Allocator & List
    frameCtx->commandAllocator->Reset();
    g_pd3dCommandList->Reset(frameCtx->commandAllocator.Get(), nullptr);

    // 3. Transition Backbuffer to RENDER_TARGET
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[g_FrameIndex].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // 4. Bind Descriptor Heaps & Set Command List on ViewportRenderer
    ID3D12DescriptorHeap* heaps[] = { g_pd3dSrvDescHeap.Get() };
    g_pd3dCommandList->SetDescriptorHeaps(1, heaps);
    EngineEditor::ViewportRenderer::Get().SetCommandList(g_pd3dCommandList.Get());

    // 5. Clear Render Target View Unconditionally
    const float clear_color_with_alpha[4] = { 0.08f, 0.08f, 0.09f, 1.00f };
    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[g_FrameIndex], clear_color_with_alpha, 0, nullptr);
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[g_FrameIndex], FALSE, nullptr);

    // 6. Start ImGui Frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (io.KeyShift) EngineEditor::CommandStack::Get().Redo();
        else EngineEditor::CommandStack::Get().Undo();
    } else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        EngineEditor::CommandStack::Get().Redo();
    }

    // Render Editor Application Shell
    EngineEditor::RenderApplicationLayout();

    // 7. Render ImGui Draw Data
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList.Get());

    // 8. Transition Backbuffer to PRESENT & Close Command List
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    // 9. Execute Command Lists
    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)g_pd3dCommandList.GetAddressOf());

    // Multi-viewport Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, (void*)g_pd3dCommandList.Get());
    }

    // 10. Present & EndFrame
    g_pSwapChain->Present(1, 0);

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_pFence.Get(), fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtx->fenceValue = fenceValue;

    s_InFrameRender = false;
}

static LRESULT CALLBACK EditorWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT:
    case WM_MOVE:
    case WM_SIZE:
    case WM_MOVING:
    case WM_SIZING:
    case WM_ENTERSIZEMOVE:
        RenderFrame();
        break;
    case WM_EXITSIZEMOVE:
        FlushCommandQueue();
        RenderFrame();
        break;
    case WM_NCHITTEST: {
        POINT pt = { (SHORT)LOWORD(lParam), (SHORT)HIWORD(lParam) };
        ScreenToClient(hwnd, &pt);
        float titleHeight = EngineEditor::GetTitleBarTotalHeight();
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (pt.y >= 0 && pt.y <= titleHeight && pt.x >= 0 && pt.x < (rect.right - 132)) {
            return HTCAPTION;
        }
        break;
    }
    }
    return CallWindowProc(g_PrevWndProc, hwnd, uMsg, wParam, lParam);
}

int main(int argc, char** argv) {
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(1920, 1080, "BLUEMAN ENGINE", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    g_Window = window;
    HWND hwnd = glfwGetWin32Window(window);
    g_PrevWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)EditorWndProc);

    if (!CreateDeviceD3D(hwnd)) {
        std::cerr << "Failed to initialize Direct3D 12 device!" << std::endl;
        CleanupDeviceD3D();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = nullptr;

    // Setup Platform/Renderer Backends BEFORE loading fonts / applying theme
    ImGui_ImplGlfw_InitForOther(window, true);
    
    g_srvHeapNextFreeIndex = 0;

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = g_pd3dDevice.Get();
    init_info.CommandQueue = g_pd3dCommandQueue.Get();
    init_info.NumFramesInFlight = NUM_FRAMES_IN_FLIGHT;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap.Get();
    init_info.SrvDescriptorAllocFn = SrvDescriptorAlloc;
    init_info.SrvDescriptorFreeFn = SrvDescriptorFree;

    ImGui_ImplDX12_Init(&init_info);

    // Apply Dark Theme & Load Fonts (now that backend capabilities are registered)
    EngineEditor::ApplyTheme();

    EngineEditor::InitializeApplication();
    EngineEditor::SetCustomTitleBarWindow(window);

    // Allocate SRV descriptor for ViewportRenderer using the dynamic allocator
    D3D12_CPU_DESCRIPTOR_HANDLE viewportCpuDesc;
    D3D12_GPU_DESCRIPTOR_HANDLE viewportGpuDesc;
    SrvDescriptorAlloc(&init_info, &viewportCpuDesc, &viewportGpuDesc);

    // Initialize ViewportRenderer with DX12 SRV handle
    EngineEditor::ViewportRenderer::Get().Init(
        g_pd3dDevice.Get(),
        g_pd3dSrvDescHeap.Get(),
        1,
        viewportCpuDesc,
        viewportGpuDesc
    );

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        RenderFrame();
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    EngineEditor::ShutdownApplication();
    EngineEditor::ViewportRenderer::Get().Shutdown();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

static bool CreateDeviceD3D(HWND hWnd) {
    // Create DXGI Factory
    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;

    // Create D3D12 Device
    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&g_pd3dDevice)))) return false;

    // Create Command Queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    if (FAILED(g_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&g_pd3dCommandQueue)))) return false;

    // Create Command Allocators & List
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_FrameContext[i].commandAllocator))))
            return false;
    }
    if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_FrameContext[0].commandAllocator.Get(), nullptr, IID_PPV_ARGS(&g_pd3dCommandList))) || FAILED(g_pd3dCommandList->Close()))
        return false;

    // Create Fence
    if (FAILED(g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_pFence)))) return false;
    g_hFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (!g_hFenceEvent) return false;

    // Create Swap Chain
    DXGI_SWAP_CHAIN_DESC1 sd = {};
    sd.BufferCount = NUM_BACK_BUFFERS;
    sd.Width = 0;
    sd.Height = 0;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    sd.Scaling = DXGI_SCALING_STRETCH;

    ComPtr<IDXGISwapChain1> swapChain1;
    if (FAILED(factory->CreateSwapChainForHwnd(g_pd3dCommandQueue.Get(), hWnd, &sd, nullptr, nullptr, &swapChain1))) return false;
    if (FAILED(swapChain1.As(&g_pSwapChain))) return false;
    g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);

    // Create RTV Descriptor Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = NUM_BACK_BUFFERS;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)))) return false;

    // Create SRV Descriptor Heap
    D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
    srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvDesc.NumDescriptors = 64;
    srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)))) return false;

    RECT rect = {};
    GetClientRect(hWnd, &rect);
    g_SwapChainWidth = (UINT)(rect.right - rect.left);
    g_SwapChainHeight = (UINT)(rect.bottom - rect.top);

    CreateRenderTarget();
    return true;
}


static void CreateRenderTarget() {
    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        g_mainRenderTargetDescriptor[i] = rtvHandle;
        g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_mainRenderTargetResource[i]));
        g_pd3dDevice->CreateRenderTargetView(g_mainRenderTargetResource[i].Get(), nullptr, rtvHandle);
        rtvHandle.ptr += rtvDescriptorSize;
    }
}

static void CleanupRenderTarget() {
    for (UINT i = 0; i < NUM_BACK_BUFFERS; i++) {
        g_mainRenderTargetResource[i].Reset();
    }
}

static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_hFenceEvent) { CloseHandle(g_hFenceEvent); g_hFenceEvent = NULL; }
    g_pFence.Reset();
    g_pd3dCommandList.Reset();
    for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++) {
        g_FrameContext[i].commandAllocator.Reset();
    }
    g_pd3dSrvDescHeap.Reset();
    g_pd3dRtvDescHeap.Reset();
    g_pSwapChain.Reset();
    g_pd3dCommandQueue.Reset();
    g_pd3dDevice.Reset();
}

static void WaitForLastSubmittedFrame() {
    FrameContext* frameCtx = &g_FrameContext[g_FrameIndex % NUM_FRAMES_IN_FLIGHT];
    UINT64 fenceValue = frameCtx->fenceValue;
    if (fenceValue == 0) return;
    frameCtx->fenceValue = 0;
    if (g_pFence->GetCompletedValue() < fenceValue) {
        g_pFence->SetEventOnCompletion(fenceValue, g_hFenceEvent);
        WaitForSingleObject(g_hFenceEvent, INFINITE);
    }
}

static FrameContext* WaitForNextFrameResources() {
    UINT nextFrameIndex = (g_FrameIndex + 1) % NUM_FRAMES_IN_FLIGHT;
    FrameContext* frameCtx = &g_FrameContext[nextFrameIndex];
    UINT64 fenceValue = frameCtx->fenceValue;
    if (fenceValue != 0) {
        frameCtx->fenceValue = 0;
        if (g_pFence->GetCompletedValue() < fenceValue) {
            g_pFence->SetEventOnCompletion(fenceValue, g_hFenceEvent);
            WaitForSingleObject(g_hFenceEvent, INFINITE);
        }
    }
    return frameCtx;
}

