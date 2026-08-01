#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H

#include <string>
#include <cstdint>
#include <d3d12.h>
#include <wrl/client.h>

namespace EngineEditor {

class SplashScreen {
public:
    static SplashScreen& Get();

    SplashScreen();
    ~SplashScreen();

    void Init(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12DescriptorHeap* srvHeap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle);
    void Shutdown();

    bool IsActive() const { return m_Active; }
    void SetActive(bool active) { m_Active = active; }
    uint64_t GetTextureID() const { return m_SRVGpuHandle.ptr; }

    void SetProgress(float progress, const std::string& statusText) {
        m_Progress = progress;
        m_StatusText = statusText;
    }
    float GetProgress() const { return m_Progress; }
    const std::string& GetStatus() const { return m_StatusText; }

private:
    void LoadSplashTexture();

    ID3D12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_CommandQueue = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRVCpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_SRVGpuHandle{};
    Microsoft::WRL::ComPtr<ID3D12Resource> m_TextureResource;

    bool m_Active = true;
    bool m_TextureLoaded = false;
    uint32_t m_TextureWidth = 0;
    uint32_t m_TextureHeight = 0;

    float m_Progress = 0.0f;
    std::string m_StatusText = "Loading Engine Subsystems...";
};

void RenderSplashScreenUI();

} // namespace EngineEditor

#endif // SPLASH_SCREEN_H
