#ifndef ZEGFX_ADAPTER_H
#define ZEGFX_ADAPTER_H

#include <stdint.h>
#include <windows.h>
#include <d3d12.h>
#include <memory>
#include <string>
#include <vector>
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

    bool IsInitialized() const { return m_Initialized; }
    zephysics::PhysicsWorld* GetPhysicsWorld() { return m_PhysicsWorld.get(); }

private:
    bool m_Initialized = false;
    uint32_t m_Width = 1280;
    uint32_t m_Height = 720;
    float m_TimeAccumulator = 0.0f;
    std::unique_ptr<zegfx::Renderer> m_Renderer;
    std::unique_ptr<zephysics::PhysicsWorld> m_PhysicsWorld;

    uint64_t m_DefaultMaterialHandle = 0;
    uint64_t m_DefaultMeshHandle = 0;

    std::vector<zegfx::RenderInstance> m_OpaqueInstances;
    std::vector<zegfx::LocalLightData> m_LocalLights;
    std::vector<zegfx::DirectionalLightData> m_DirectionalLights;
};

} // namespace EngineEditor

#endif // ZEGFX_ADAPTER_H
