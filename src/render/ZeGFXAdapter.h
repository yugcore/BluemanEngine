#ifndef ZEGFX_ADAPTER_H
#define ZEGFX_ADAPTER_H

#include <stdint.h>
#include <windows.h>
#include <d3d12.h>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "render_scene_snapshot.h"

namespace zegfx {
    class Renderer;
}

namespace zephysics {
    class PhysicsWorld;
}

namespace EngineEditor {

class ZeGFXAdapter {
public:
    static ZeGFXAdapter& Get();

    ZeGFXAdapter();
    ~ZeGFXAdapter();

    bool Initialize(ID3D12Device* device, HWND hwnd, uint32_t width, uint32_t height);
    void Shutdown();
    void Resize(uint32_t width, uint32_t height);
    void Render(ID3D12GraphicsCommandList* cmdList, uint32_t width, uint32_t height, float deltaTime);
    void SyncEngineState(float deltaTime = 0.016f);

    // GPU Asset Bridge methods
    zegfx::RenderMeshHandle LoadMeshAsset(const std::string& meshPath);
    zegfx::RenderMaterialHandle LoadMaterialAsset(const std::string& matPath);
    void CreateDefaultPrimitives();

    // Render target redirection
    void SetOutputRenderTarget(ID3D12Resource* rtResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);

    bool IsInitialized() const { return m_Initialized; }
    zephysics::PhysicsWorld* GetPhysicsWorld() { return m_PhysicsWorld.get(); }

private:
    bool m_Initialized = false;
    uint32_t m_Width = 1280;
    uint32_t m_Height = 720;
    float m_TimeAccumulator = 0.0f;
    std::unique_ptr<zegfx::Renderer> m_Renderer;
    std::unique_ptr<zephysics::PhysicsWorld> m_PhysicsWorld;

    ID3D12Resource* m_ActiveRenderTargetResource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ActiveRtvHandle{};

    zegfx::RenderMaterialHandle m_DefaultMaterialHandle = {};
    zegfx::RenderMeshHandle m_DefaultMeshHandle = {};

    std::unordered_map<std::string, zegfx::RenderMeshHandle> m_LoadedMeshes;
    std::unordered_map<std::string, zegfx::RenderMaterialHandle> m_LoadedMaterials;

    std::vector<zegfx::RenderInstance> m_OpaqueInstances;
    std::vector<zegfx::LocalLightData> m_LocalLights;
    std::vector<zegfx::DirectionalLightData> m_DirectionalLights;
};

} // namespace EngineEditor

#endif // ZEGFX_ADAPTER_H
