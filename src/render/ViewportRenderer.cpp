#include "ViewportRenderer.h"
#include "ZeGFXAdapter.h"
#include "DX12Host.h"
#include <iostream>
#include <cmath>

using Microsoft::WRL::ComPtr;

namespace EngineEditor {


ViewportRenderer& ViewportRenderer::Get() {
    static ViewportRenderer instance;
    return instance;
}

ViewportRenderer::ViewportRenderer() {
}

ViewportRenderer::~ViewportRenderer() {
    Shutdown();
}

void ViewportRenderer::Init(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle) {
    m_Device = device;
    m_SrvHeap = srvHeap;
    m_SrvDescriptorIndex = srvDescriptorIndex;
    m_SRVCpuHandle = srvCpuHandle;
    m_SRVGpuHandle = srvGpuHandle;

    if (!m_Device) return;

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap)))) {
        std::cerr << "[ViewportRenderer] Failed to create RTV descriptor heap!" << std::endl;
        return;
    }
    m_RtvHandle = m_RtvHeap->GetCPUDescriptorHandleForHeapStart();

    Resize(1280, 720);
}

void ViewportRenderer::Shutdown() {
    DeleteFramebuffer();
    m_RtvHeap.Reset();
}

void ViewportRenderer::DeleteFramebuffer() {
    if (m_ColorTexture) {
        WaitForGPU();
        m_ColorTexture.Reset();
    }
}


void ViewportRenderer::CreateFramebuffer(uint32_t width, uint32_t height) {
    DeleteFramebuffer();

    m_Width = width;
    m_Height = height;

    if (m_Width == 0 || m_Height == 0 || !m_Device) return;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = m_Width;
    resDesc.Height = m_Height;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearVal = {};
    clearVal.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearVal.Color[0] = 0.10f;
    clearVal.Color[1] = 0.12f;
    clearVal.Color[2] = 0.16f;
    clearVal.Color[3] = 1.00f;

    m_CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

    HRESULT hr = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        m_CurrentState,
        &clearVal,
        IID_PPV_ARGS(&m_ColorTexture)
    );

    if (FAILED(hr)) {
        std::cerr << "[ViewportRenderer] Failed to create offscreen render target texture!" << std::endl;
        return;
    }

    m_Device->CreateRenderTargetView(m_ColorTexture.Get(), nullptr, m_RtvHandle);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    m_Device->CreateShaderResourceView(m_ColorTexture.Get(), &srvDesc, m_SRVCpuHandle);
}

void ViewportRenderer::Resize(uint32_t width, uint32_t height) {
    if (width == m_Width && height == m_Height) return;
    if (width == 0 || height == 0) return;
    CreateFramebuffer(width, height);
}

void ViewportRenderer::RenderScene(float deltaTime, ID3D12GraphicsCommandList* cmdList) {
    ID3D12GraphicsCommandList* cl = cmdList ? cmdList : m_ActiveCmdList;
    if (m_Width == 0 || m_Height == 0 || !cl) return;

    if (m_RenderCallback) {
        m_RenderCallback(cl, m_Width, m_Height, deltaTime);
        return;
    }

    if (!m_ColorTexture) return;

    // Transition offscreen target to RENDER_TARGET state
    if (m_CurrentState != D3D12_RESOURCE_STATE_RENDER_TARGET) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_ColorTexture.Get();
        barrier.Transition.StateBefore = m_CurrentState;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cl->ResourceBarrier(1, &barrier);
        m_CurrentState = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }

    // Clear offscreen render target with atmospheric Blue Sky clear color
    float skyClearColor[4] = { 0.38f, 0.62f, 0.92f, 1.00f };
    cl->ClearRenderTargetView(m_RtvHandle, skyClearColor, 0, nullptr);

    // Execute ZeGFX Engine Rendering Pipeline
    ZeGFXAdapter::Get().Render(cl, m_Width, m_Height, deltaTime);

    // Transition offscreen target back to PIXEL_SHADER_RESOURCE state for ImGui composition
    if (m_CurrentState != D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_ColorTexture.Get();
        barrier.Transition.StateBefore = m_CurrentState;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        cl->ResourceBarrier(1, &barrier);
        m_CurrentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    }
}

} // namespace EngineEditor

