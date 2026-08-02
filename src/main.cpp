#include <iostream>
#include <vector>
#include <chrono>
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
#include "render/ZeGFXAdapter.h"
#include "render/SplashScreen.h"
#include "core/BackgroundAssetCooker.h"
#include "core/AssetRegistry.h"
#include "core/WindowsFileDialog.h"

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

static constexpr UINT kMaxSrvDescriptors = 4096;
static UINT g_srvHeapNextFreeIndex = 0;
static std::vector<UINT> g_srvFreeList;

static void SrvDescriptorAlloc(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
    UINT index = 0;
    if (!g_srvFreeList.empty()) {
        index = g_srvFreeList.back();
        g_srvFreeList.pop_back();
    } else if (g_srvHeapNextFreeIndex < kMaxSrvDescriptors) {
        index = g_srvHeapNextFreeIndex++;
    } else {
        std::cerr << "[DX12] FATAL ERROR: SRV Descriptor Heap exhausted (" << kMaxSrvDescriptors << " descriptors allocated)!" << std::endl;
        index = 0;
    }

    UINT handleIncrement = info->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = info->SrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
    
    cpuHandle.ptr += (SIZE_T)index * handleIncrement;
    gpuHandle.ptr += (SIZE_T)index * handleIncrement;
    
    *out_cpu_handle = cpuHandle;
    *out_gpu_handle = gpuHandle;
}

static void SrvDescriptorFree(ImGui_ImplDX12_InitInfo* info, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) {
    (void)gpu_handle;
    if (!info || !info->Device || !info->SrvDescriptorHeap || cpu_handle.ptr == 0) return;

    UINT handleIncrement = info->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    SIZE_T heapStart = info->SrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;

    if (cpu_handle.ptr >= heapStart && handleIncrement > 0) {
        UINT index = static_cast<UINT>((cpu_handle.ptr - heapStart) / handleIncrement);
        if (index < kMaxSrvDescriptors) {
            g_srvFreeList.push_back(index);
        }
    }
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
static double s_LastFrameTime = 0.0;
static uint64_t s_FrameCounter = 0;

static void RenderFrame() {
    if (s_InFrameRender || !g_Window || !g_pSwapChain) return;
    s_InFrameRender = true;

    auto tFrameStart = std::chrono::high_resolution_clock::now();

    double currentTime = glfwGetTime();
    float deltaTime = (s_LastFrameTime > 0.0) ? static_cast<float>(currentTime - s_LastFrameTime) : 0.01667f;
    if (deltaTime <= 0.0f || deltaTime > 0.1f) deltaTime = 0.01667f;
    s_LastFrameTime = currentTime;

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
    auto t1 = std::chrono::high_resolution_clock::now();
    FrameContext* frameCtx = WaitForNextFrameResources();
    g_FrameIndex = g_pSwapChain->GetCurrentBackBufferIndex();
    auto tWait = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t1).count();

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

    // 6. Start ImGui Frame & Render Application Layout
    auto t2 = std::chrono::high_resolution_clock::now();
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiIO& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        if (io.KeyShift) EngineEditor::CommandStack::Get().Redo();
        else EngineEditor::CommandStack::Get().Undo();
    } else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        EngineEditor::CommandStack::Get().Redo();
    } else if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_I, false)) {
        EngineEditor::EditorState::Get().TriggerImportFileDialog();
    }

    // Render Editor Application Shell (includes Viewport & ZeGFX Adapter rendering)
    EngineEditor::RenderApplicationLayout();

    // Check if import file dialog was requested
    if (EngineEditor::EditorState::Get().requestImportFileDialog) {
        EngineEditor::EditorState::Get().requestImportFileDialog = false;
        auto selectedFiles = EngineEditor::WindowsFileDialog::OpenFileDialog(
            EngineEditor::FileDialogType::ImportAsset,
            "Import Assets into Blueman Engine",
            true
        );
        if (!selectedFiles.empty()) {
            EngineEditor::BackgroundAssetCooker::Get().QueueFilesForCooking(selectedFiles);
        }
    }

    // Render background asset cooking progress overlay & notifications
    EngineEditor::BackgroundAssetCooker::Get().RenderCookingOverlay();

    auto tLayout = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t2).count();

    // 7. Re-bind Main Window Backbuffer RTV & Viewport before ImGui draw data rendering
    D3D12_VIEWPORT mainViewport = { 0.0f, 0.0f, (float)display_w, (float)display_h, 0.0f, 1.0f };
    D3D12_RECT mainScissor = { 0, 0, display_w, display_h };
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[g_FrameIndex], FALSE, nullptr);
    g_pd3dCommandList->RSSetViewports(1, &mainViewport);
    g_pd3dCommandList->RSSetScissorRects(1, &mainScissor);

    auto t3 = std::chrono::high_resolution_clock::now();
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList.Get());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, (void*)g_pd3dCommandList.Get());
    }
    auto tImGuiDraw = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t3).count();

    // 8. Transition Backbuffer to PRESENT & Close Command List
    auto t4 = std::chrono::high_resolution_clock::now();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);
    g_pd3dCommandList->Close();

    // 9. Execute Command Lists
    g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)g_pd3dCommandList.GetAddressOf());
    auto tExecute = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t4).count();

    // 10. Present & EndFrame
    auto t5 = std::chrono::high_resolution_clock::now();
    UINT syncInterval = EngineEditor::EditorState::Get().settings.enableVSync ? 1 : 0;
    g_pSwapChain->Present(syncInterval, 0);

    UINT64 fenceValue = g_fenceLastSignaledValue + 1;
    g_pd3dCommandQueue->Signal(g_pFence.Get(), fenceValue);
    g_fenceLastSignaledValue = fenceValue;
    frameCtx->fenceValue = fenceValue;
    s_InFrameRender = false;

    // --- High-Resolution Profiling Instrumentation ---
    auto tEnd = std::chrono::high_resolution_clock::now();
    double tTotalMs = std::chrono::duration<double, std::milli>(tEnd - tFrameStart).count();
    double tPresentMs = std::chrono::duration<double, std::milli>(tEnd - t5).count();

    static uint64_t s_ProfileFrameCount = 0;
    static double s_AccTotalMs = 0.0;
    static double s_AccWaitMs = 0.0;
    static double s_AccLayoutMs = 0.0;
    static double s_AccImGuiDrawMs = 0.0;
    static double s_AccExecuteMs = 0.0;
    static double s_AccPresentMs = 0.0;

    s_ProfileFrameCount++;
    s_AccTotalMs += tTotalMs;
    s_AccWaitMs += tWait;
    s_AccLayoutMs += tLayout;
    s_AccImGuiDrawMs += tImGuiDraw;
    s_AccExecuteMs += tExecute;
    s_AccPresentMs += tPresentMs;

    if (s_ProfileFrameCount % 120 == 0) {
        double avgTotal = s_AccTotalMs / 120.0;
        double avgWait = s_AccWaitMs / 120.0;
        double avgLayout = s_AccLayoutMs / 120.0;
        double avgImGuiDraw = s_AccImGuiDrawMs / 120.0;
        double avgExecute = s_AccExecuteMs / 120.0;
        double avgPresent = s_AccPresentMs / 120.0;
        double avgFps = (avgTotal > 0.001) ? (1000.0 / avgTotal) : 0.0;

        std::cout << "[PROFILE METRICS] Frames: " << s_ProfileFrameCount
                  << " | FPS: " << avgFps
                  << " | TotalFrame: " << avgTotal << " ms"
                  << " | WaitNextFrameRes: " << avgWait << " ms"
                  << " | Layout&ZeGFX: " << avgLayout << " ms"
                  << " | ImGuiRender: " << avgImGuiDraw << " ms"
                  << " | ExecuteCmds: " << avgExecute << " ms"
                  << " | Present: " << avgPresent << " ms"
                  << std::endl;

        s_AccTotalMs = 0.0; s_AccWaitMs = 0.0; s_AccLayoutMs = 0.0;
        s_AccImGuiDrawMs = 0.0; s_AccExecuteMs = 0.0; s_AccPresentMs = 0.0;
    }
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
        if (pt.y >= 0 && pt.y <= titleHeight) {
            if (ImGui::GetCurrentContext() != nullptr) {
                ImGuiIO& io = ImGui::GetIO();
                if (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopupId) || io.WantCaptureMouse) {
                    return HTCLIENT;
                }
            }
            if (pt.x >= (rect.right - 132)) {
                return HTCLIENT;
            }
            return HTCAPTION;
        }
        break;
    }
    }
    return CallWindowProc(g_PrevWndProc, hwnd, uMsg, wParam, lParam);
}

int main(int argc, char** argv) {
    DisableProcessWindowsGhosting();
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // ------------------------------------------------------------------------
    // PHASE 1: Standalone Unreal-Style Splash Loading Window (640x360)
    // ------------------------------------------------------------------------
    const GLFWvidmode* videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int splashW = 640;
    int splashH = 360;
    int splashX = (videoMode->width - splashW) / 2;
    int splashY = (videoMode->height - splashH) / 2;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* splashWindow = glfwCreateWindow(splashW, splashH, "Blueman Engine Loading...", nullptr, nullptr);
    if (splashWindow) {
        g_Window = splashWindow;
        glfwSetWindowPos(splashWindow, splashX, splashY);
        HWND splashHwnd = glfwGetWin32Window(splashWindow);
        g_PrevWndProc = (WNDPROC)SetWindowLongPtr(splashHwnd, GWLP_WNDPROC, (LONG_PTR)EditorWndProc);

        if (CreateDeviceD3D(splashHwnd)) {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& splashIo = ImGui::GetIO(); (void)splashIo;
            splashIo.IniFilename = nullptr;

            ImGui_ImplGlfw_InitForOther(splashWindow, true);
            g_srvHeapNextFreeIndex = 0;
            g_srvFreeList.clear();

            ImGui_ImplDX12_InitInfo splashInitInfo = {};
            splashInitInfo.Device = g_pd3dDevice.Get();
            splashInitInfo.CommandQueue = g_pd3dCommandQueue.Get();
            splashInitInfo.NumFramesInFlight = NUM_FRAMES_IN_FLIGHT;
            splashInitInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
            splashInitInfo.SrvDescriptorHeap = g_pd3dSrvDescHeap.Get();
            splashInitInfo.SrvDescriptorAllocFn = SrvDescriptorAlloc;
            splashInitInfo.SrvDescriptorFreeFn = SrvDescriptorFree;
            ImGui_ImplDX12_Init(&splashInitInfo);

            EngineEditor::ApplyTheme();

            D3D12_CPU_DESCRIPTOR_HANDLE splashCpuDesc;
            D3D12_GPU_DESCRIPTOR_HANDLE splashGpuDesc;
            SrvDescriptorAlloc(&splashInitInfo, &splashCpuDesc, &splashGpuDesc);

            EngineEditor::SplashScreen::Get().Init(
                g_pd3dDevice.Get(),
                g_pd3dCommandQueue.Get(),
                g_pd3dSrvDescHeap.Get(),
                splashCpuDesc,
                splashGpuDesc
            );

            struct LoadStep {
                float progress;
                const char* status;
            };
            LoadStep loadSteps[] = {
                { 0.15f, "Initializing Direct3D 12 Hardware Interface..." },
                { 0.35f, "Loading ZeGFX Engine Backends (DX12)..." },
                { 0.60f, "Prewarming HLSL Shader Variants & Pipeline Cache..." },
                { 0.85f, "Synchronizing Scene Graph & Render World..." },
                { 1.00f, "Launching BluemanEngine Editor..." }
            };

            for (int i = 0; i < 5; i++) {
                EngineEditor::SplashScreen::Get().SetProgress(loadSteps[i].progress, loadSteps[i].status);
                double startTime = glfwGetTime();
                while (glfwGetTime() - startTime < 0.8 && !glfwWindowShouldClose(splashWindow)) {
                    glfwPollEvents();
                    RenderFrame();
                }
            }

            WaitForLastSubmittedFrame();
            EngineEditor::SplashScreen::Get().Shutdown();
            ImGui_ImplDX12_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            CleanupRenderTarget();
            g_pSwapChain.Reset();
            SetWindowLongPtr(splashHwnd, GWLP_WNDPROC, (LONG_PTR)g_PrevWndProc);
            g_Window = nullptr;
        }
        glfwDestroyWindow(splashWindow);
    }

    // ------------------------------------------------------------------------
    // PHASE 2: Main Blueman Engine Editor Window Initialization & Reveal
    // ------------------------------------------------------------------------
    EngineEditor::SplashScreen::Get().SetActive(false);

    int editorW = 1920;
    int editorH = 1080;
    if (videoMode->width < 1920 || videoMode->height < 1080) {
        editorW = videoMode->width - 100;
        editorH = videoMode->height - 100;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(editorW, editorH, "BLUEMAN ENGINE", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetWindowPos(window, (videoMode->width - editorW) / 2, (videoMode->height - editorH) / 2);

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

    // Initialize Dear ImGui for Main Editor
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.IniFilename = "blueman_layout.ini";

    ImGui_ImplGlfw_InitForOther(window, true);
    
    g_srvHeapNextFreeIndex = 0;
    g_srvFreeList.clear();

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = g_pd3dDevice.Get();
    init_info.CommandQueue = g_pd3dCommandQueue.Get();
    init_info.NumFramesInFlight = NUM_FRAMES_IN_FLIGHT;
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    init_info.SrvDescriptorHeap = g_pd3dSrvDescHeap.Get();
    init_info.SrvDescriptorAllocFn = SrvDescriptorAlloc;
    init_info.SrvDescriptorFreeFn = SrvDescriptorFree;

    ImGui_ImplDX12_Init(&init_info);

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

    // Initialize ZeGFX Engine Adapter
    EngineEditor::ZeGFXAdapter::Get().Initialize(
        g_pd3dDevice.Get(),
        hwnd,
        1280,
        720
    );

    // Initial scan of cooked asset directory
    EngineEditor::AssetRegistry::Get().ScanProjectFolder("Z:\\Blueman Cooked Assets");

    // Register GLFW drag-and-drop file import callback
    glfwSetDropCallback(window, [](GLFWwindow* w, int count, const char** paths) {
        (void)w;
        std::vector<std::string> filePaths;
        for (int i = 0; i < count; i++) {
            if (paths[i] && paths[i][0] != '\0') {
                filePaths.push_back(paths[i]);
            }
        }
        if (!filePaths.empty()) {
            EngineEditor::BackgroundAssetCooker::Get().QueueFilesForCooking(filePaths);
        }
    });

    // Main Loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        RenderFrame();
    }

    WaitForLastSubmittedFrame();

    // Cleanup
    EngineEditor::ZeGFXAdapter::Get().Shutdown();
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
    if (!g_pd3dDevice) {
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

        // Create RTV Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
        rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDesc.NumDescriptors = NUM_BACK_BUFFERS;
        rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)))) return false;

        // Create SRV Descriptor Heap
        D3D12_DESCRIPTOR_HEAP_DESC srvDesc = {};
        srvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvDesc.NumDescriptors = kMaxSrvDescriptors;
        srvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&srvDesc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)))) return false;
    }

    // Attach Swap Chain for target hWnd
    CleanupRenderTarget();
    g_pSwapChain.Reset();

    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) return false;

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

