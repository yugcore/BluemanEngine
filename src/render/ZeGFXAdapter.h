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
#include "heightmap_importer.h"

namespace zegfx {
    class Renderer;
}

namespace zephysics {
    class PhysicsWorld;
}

namespace EngineEditor {

// Phase 4: Decoupled Render Proxy POD Snapshot
struct DirectionalLightProxy {
    float direction[3] = { 0.0f, -1.0f, 0.0f };
    float color[3] = { 1.0f, 0.95f, 0.85f };
    float illuminanceLux = 100000.0f;
    float angularDiameterRadians = 0.0093f;
    bool castShadows = true;
    int cascadeCount = 4;
    float shadowDistance = 200.0f;
    float cascadeDistributionExponent = 0.5f;
    uint32_t flags = 1u;
};

class ZeGFXAdapter {
public:
    static ZeGFXAdapter& Get();

    ZeGFXAdapter();
    ~ZeGFXAdapter();

    bool Initialize(ID3D12Device* device, HWND hwnd, uint32_t width, uint32_t height);
    void Shutdown();
    void Resize(uint32_t width, uint32_t height);
    void Render(ID3D12GraphicsCommandList* cmdList, uint32_t width, uint32_t height, float deltaTime);
    void SyncEngineState(float deltaTime = 0.016f, zegfx::ExternalCmdListHandle externalCmdList = {});

    // GPU Asset Bridge methods
    zegfx::RenderMeshHandle LoadMeshAsset(const std::string& meshPath);
    zegfx::RenderMaterialHandle LoadMaterialAsset(const std::string& matPath);
    std::string CreateTerrainFromHeightmap(const std::string& name, const std::string& filePath, const zegfx::HeightmapImportSettings& settings, std::string& outError);
    void CreateDefaultPrimitives();
    void SetLightingDebugMode(int mode);

    // Render target redirection & Clear API
    void SetOutputRenderTarget(ID3D12Resource* rtResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle);
    void Clear(zegfx::Color color = zegfx::Color(51, 107, 191, 255));
    zegfx::Color GetSkyColor() const;

    bool IsInitialized() const { return m_Initialized; }
    zephysics::PhysicsWorld* GetPhysicsWorld() { return m_PhysicsWorld.get(); }

private:
    bool m_Initialized = false;
    uint32_t m_Width = 1280;
    uint32_t m_Height = 720;
    float m_TimeAccumulator = 0.0f;
    float m_PhysicsAccumulator = 0.0f;
    std::unique_ptr<zegfx::Renderer> m_Renderer;
    std::unique_ptr<zephysics::PhysicsWorld> m_PhysicsWorld;

    ID3D12Resource* m_ActiveRenderTargetResource = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_ActiveRtvHandle{};

    zegfx::RenderMaterialHandle m_DefaultMaterialHandle = {};
    zegfx::RenderMeshHandle m_DefaultMeshHandle = {};
    zegfx::RenderMaterialHandle m_DefaultPlaneMaterialHandle = {};
    zegfx::RenderMeshHandle m_DefaultPlaneMeshHandle = {};
    zegfx::RenderMeshHandle m_DefaultSphereMeshHandle = {};
    zegfx::RenderMeshHandle m_DefaultCylinderMeshHandle = {};
    zegfx::RenderMeshHandle m_DefaultConeMeshHandle = {};
    zegfx::RenderMeshHandle m_DefaultTerrain32x32MeshHandle = {};

    std::unordered_map<std::string, zegfx::RenderMeshHandle> m_LoadedMeshes;
    std::unordered_map<std::string, zegfx::RenderMaterialHandle> m_LoadedMaterials;

    std::vector<zegfx::RenderInstance> m_OpaqueInstances;
    std::vector<zegfx::LocalLightData> m_LocalLights;
    std::vector<zegfx::DirectionalLightData> m_DirectionalLights;
    zegfx::Color m_SkyAmbientColor = zegfx::Color(140, 170, 205, 255);
};

} // namespace EngineEditor

#endif // ZEGFX_ADAPTER_H
