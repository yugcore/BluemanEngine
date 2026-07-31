#ifndef VIEWPORT_RENDERER_H
#define VIEWPORT_RENDERER_H

#include <stdint.h>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace EngineEditor {

class ViewportRenderer {
public:
    static ViewportRenderer& Get();

    ViewportRenderer();
    ~ViewportRenderer();

    void Init(ID3D12Device* device, ID3D12DescriptorHeap* srvHeap, UINT srvDescriptorIndex, D3D12_CPU_DESCRIPTOR_HANDLE srvCpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE srvGpuHandle);
    void Shutdown();
    void Resize(uint32_t width, uint32_t height);
    void SetCommandList(ID3D12GraphicsCommandList* cmdList) { m_ActiveCmdList = cmdList; }
    void RenderScene(float deltaTime, ID3D12GraphicsCommandList* cmdList = nullptr);

    uint64_t GetTextureID() const { return m_SRVGpuHandle.ptr; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }

private:
    void CreateFramebuffer(uint32_t width, uint32_t height);
    void DeleteFramebuffer();

    ID3D12Device* m_Device = nullptr;
    ID3D12DescriptorHeap* m_SrvHeap = nullptr;
    UINT m_SrvDescriptorIndex = 0;
    ID3D12GraphicsCommandList* m_ActiveCmdList = nullptr;

    ComPtr<ID3D12Resource> m_ColorTexture;
    ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_RtvHandle{};
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRVCpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_SRVGpuHandle{};

    uint32_t m_Width = 0;
    uint32_t m_Height = 0;
    float m_RotationAngle = 0.0f;
    D3D12_RESOURCE_STATES m_CurrentState = D3D12_RESOURCE_STATE_COMMON;
};

} // namespace EngineEditor

#endif // VIEWPORT_RENDERER_H

