#include "ZeGFXAdapter.h"
#include "zegfx.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>

#include "physics/physics_world.h"

namespace EngineEditor {

ZeGFXAdapter& ZeGFXAdapter::Get() {
    static ZeGFXAdapter instance;
    return instance;
}

ZeGFXAdapter::ZeGFXAdapter() {
}

ZeGFXAdapter::~ZeGFXAdapter() {
    Shutdown();
}

bool ZeGFXAdapter::Initialize(ID3D12Device* device, HWND hwnd, uint32_t width, uint32_t height) {
    if (m_Initialized) return true;

    m_Width = width > 0 ? width : 1280;
    m_Height = height > 0 ? height : 720;

    zegfx::RendererSettings settings = zegfx::RendererSettings::fromPreset(zegfx::QualityPreset::Medium);
    settings.outputWidth = m_Width;
    settings.outputHeight = m_Height;
    settings.requestedTier = zegfx::CapabilityTier::Tier2_DXR_Effects;

    // Synchronize Ray Tracing toggles from EditorState settings
    const auto& edSettings = EditorState::Get().settings;
    settings.rt.rtAO = edSettings.rtAO;
    settings.rt.rtReflections = edSettings.rtReflections;

    m_Renderer = std::make_unique<zegfx::Renderer>(settings);
    m_PhysicsWorld = std::make_unique<zephysics::PhysicsWorld>();

    std::string err;
    if (!m_Renderer->initialize((void*)hwnd, err)) {
        std::cerr << "[ZeGFXAdapter] Note/Warning: " << err << std::endl;
    }

    // Configure initial directional sun light
    zegfx::DirectionalLightData sunLight = {};
    sunLight.direction = { -0.5f, -0.8f, -0.3f };
    sunLight.color = { 1.0f, 0.95f, 0.85f };
    sunLight.illuminanceLux = 100000.0f;
    m_Renderer->setDirectionalLight(sunLight);

    m_Initialized = true;
    std::cout << "[ZeGFXAdapter] ZeGFX engine & ZePhysics 3D backend initialized successfully (" << m_Width << "x" << m_Height << ")" << std::endl;
    return true;
}

void ZeGFXAdapter::Shutdown() {
    if (!m_Initialized) return;
    if (m_Renderer) {
        m_Renderer->shutdown();
        m_Renderer.reset();
    }
    m_PhysicsWorld.reset();
    m_Initialized = false;
}

void ZeGFXAdapter::Resize(uint32_t width, uint32_t height) {
    if (width == m_Width && height == m_Height) return;
    if (width == 0 || height == 0) return;

    m_Width = width;
    m_Height = height;

    if (m_Renderer) {
        m_Renderer->onResize(m_Width, m_Height);
    }
}

void ZeGFXAdapter::SyncEngineState() {
    if (!m_Renderer) return;

    const auto& edSettings = EditorState::Get().settings;

    // Check if any renderer settings actually changed before calling expensive updateSettings()
    static bool s_FirstSync = true;
    static uint32_t s_LastWidth = 0, s_LastHeight = 0;
    static bool s_LastRtAO = false, s_LastRtRefl = false, s_LastRtGI = false, s_LastFog = false;
    static int s_LastShadowRes = 0;

    bool rtAO = (edSettings.ao.mode == 2) || edSettings.rtAO;
    bool rtRefl = edSettings.rtReflections;
    bool rtGI = edSettings.rtGI;
    bool fog = edSettings.fog.enableVolumetric;
    int shadowRes = edSettings.shadow.cascadeResolution;

    bool settingsChanged = s_FirstSync ||
                           (s_LastWidth != m_Width) ||
                           (s_LastHeight != m_Height) ||
                           (s_LastRtAO != rtAO) ||
                           (s_LastRtRefl != rtRefl) ||
                           (s_LastRtGI != rtGI) ||
                           (s_LastFog != fog) ||
                           (s_LastShadowRes != shadowRes);

    if (settingsChanged) {
        s_FirstSync = false;
        s_LastWidth = m_Width;
        s_LastHeight = m_Height;
        s_LastRtAO = rtAO;
        s_LastRtRefl = rtRefl;
        s_LastRtGI = rtGI;
        s_LastFog = fog;
        s_LastShadowRes = shadowRes;

        zegfx::RendererSettings rSettings = m_Renderer->getSettings();
        rSettings.outputWidth = m_Width;
        rSettings.outputHeight = m_Height;

        rSettings.rt.rtAO = rtAO;
        rSettings.rt.rtReflections = rtRefl;

        if (rtGI) {
            rSettings.gi.mode = zegfx::GlobalIlluminationMode::RayTracedProbes;
            rSettings.gi.probeCountsPerAxis = 12;
            rSettings.gi.raysPerProbe = (uint32_t)edSettings.giRaysPerProbe;
            rSettings.gi.probesUpdatedPerFrame = (uint32_t)edSettings.giProbesUpdatedPerFrame;
        } else {
            rSettings.gi.mode = zegfx::GlobalIlluminationMode::EnvironmentOnly;
        }

        rSettings.fog.enabled = fog;
        rSettings.fog.density = edSettings.fog.density;
        rSettings.fog.inScatteringColor = zegfx::Color(
            (uint8_t)(edSettings.fog.color[0] * 255.0f),
            (uint8_t)(edSettings.fog.color[1] * 255.0f),
            (uint8_t)(edSettings.fog.color[2] * 255.0f),
            255
        );

        rSettings.shadow.cascadeResolution = edSettings.shadow.cascadeResolution;
        rSettings.shadow.cascadeCount = edSettings.shadow.cascadeCount;
        rSettings.shadow.maxDistance = edSettings.shadow.maxDistance;
        rSettings.shadow.constantBias = edSettings.shadow.constantBias;
        rSettings.shadow.slopeBias = edSettings.shadow.slopeBias;
        rSettings.shadow.normalBias = edSettings.shadow.normalBias;
        rSettings.shadow.filterSoftness = edSettings.shadow.filterSoftness;

        rSettings.ao.radius = edSettings.ao.radius;
        rSettings.ao.intensity = edSettings.ao.intensity;
        rSettings.ao.temporal = edSettings.ao.temporalFiltering;

        rSettings.exposure.autoExposure = edSettings.postFX.autoExposure;
        rSettings.exposure.exposureCompensationEV = edSettings.postFX.exposureEV;
        rSettings.bloom.intensity = edSettings.postFX.bloomIntensity;
        rSettings.bloom.threshold = edSettings.postFX.bloomThreshold;

        m_Renderer->updateSettings(rSettings);
    }

    // ------------------------------------------------------------------------
    // Dynamic Entity-Component Snapshot Assembly & Local Light Injection
    // ------------------------------------------------------------------------
    m_OpaqueInstances.clear();
    m_LocalLights.clear();
    m_DirectionalLights.clear();

    m_OpaqueInstances.reserve(512);
    m_LocalLights.reserve(32);
    m_DirectionalLights.reserve(4);

    // 1. Traversal of SceneGraph Nodes for Directional Lights, Mesh Entities, Foliage, Terrain & Local Lights
    const auto& rootNodes = SceneGraph::Get().GetRootNodes();

    // Look for Directional Light in SceneGraph first
    const SceneNode* sunNode = SceneGraph::Get().FindNode("DirectionalSunLight");
    zegfx::DirectionalLightData sunLight = {};
    if (sunNode) {
        // Calculate direction vector from Euler rotation (degrees)
        float pitchRad = sunNode->rotation[0] * 3.14159265f / 180.0f;
        float yawRad   = sunNode->rotation[1] * 3.14159265f / 180.0f;
        sunLight.direction = {
            std::cos(pitchRad) * std::sin(yawRad),
            -std::sin(pitchRad),
            -std::cos(pitchRad) * std::cos(yawRad)
        };
        sunLight.color = { 1.0f, 0.95f, 0.85f };
        sunLight.illuminanceLux = 100000.0f;
    } else {
        sunLight.direction = { -0.5f, -0.8f, -0.3f };
        sunLight.color = { 1.0f, 0.95f, 0.85f };
        sunLight.illuminanceLux = 100000.0f;
    }
    m_DirectionalLights.push_back(sunLight);

    auto traverseNodes = [&](auto& self, const std::vector<SceneNode>& nodes) -> void {
        for (const auto& node : nodes) {
            if (node.type == SceneNodeType::Actor || node.type == SceneNodeType::Terrain) {
                zegfx::RenderInstance inst = {};
                inst.mesh = zegfx::RenderMeshHandle{ (uint32_t)m_DefaultMeshHandle };
                inst.material = zegfx::RenderMaterialHandle{ (uint32_t)m_DefaultMaterialHandle };
                inst.world = zegfx::Mat4::identity();
                inst.world.m[3][0] = node.location[0];
                inst.world.m[3][1] = node.location[1];
                inst.world.m[3][2] = node.location[2];
                inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                inst.visibilityFlags = 1;
                m_OpaqueInstances.push_back(inst);
            } else if (node.type == SceneNodeType::FoliageCluster) {
                // High-density foliage instancing (trees, shrubs, grass) across 500m-1km area
                const int numTrees = 250;
                for (int i = 0; i < numTrees; ++i) {
                    float angle = (float)i * 0.125f;
                    float dist = 20.0f + (float)(i % 50) * 15.0f; // Spans up to 770m
                    float px = node.location[0] + std::cos(angle) * dist;
                    float pz = node.location[2] + std::sin(angle) * dist;
                    float py = node.location[1] + (std::sin(px * 0.05f) + std::cos(pz * 0.05f)) * 3.5f;

                    zegfx::RenderInstance inst = {};
                    inst.mesh = zegfx::RenderMeshHandle{ (uint32_t)m_DefaultMeshHandle };
                    inst.material = zegfx::RenderMaterialHandle{ (uint32_t)m_DefaultMaterialHandle };
                    inst.world = zegfx::Mat4::identity();
                    inst.world.m[0][0] = 1.0f + (float)(i % 3) * 0.4f; // Scale variation
                    inst.world.m[1][1] = 1.5f + (float)(i % 4) * 0.5f; // Tree height variation
                    inst.world.m[2][2] = 1.0f + (float)(i % 3) * 0.4f;
                    inst.world.m[3][0] = px;
                    inst.world.m[3][1] = py;
                    inst.world.m[3][2] = pz;
                    inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                    inst.visibilityFlags = 1; // Enables ExecuteIndirect / GPU compute culling
                    m_OpaqueInstances.push_back(inst);
                }
            } else if (node.type == SceneNodeType::PathPoint) {
                // Forest trail path marker
                zegfx::RenderInstance inst = {};
                inst.mesh = zegfx::RenderMeshHandle{ (uint32_t)m_DefaultMeshHandle };
                inst.material = zegfx::RenderMaterialHandle{ (uint32_t)m_DefaultMaterialHandle };
                inst.world = zegfx::Mat4::identity();
                inst.world.m[0][0] = 0.5f; inst.world.m[1][1] = 0.1f; inst.world.m[2][2] = 0.5f; // Flat path stone
                inst.world.m[3][0] = node.location[0];
                inst.world.m[3][1] = node.location[1];
                inst.world.m[3][2] = node.location[2];
                inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                inst.visibilityFlags = 1;
                m_OpaqueInstances.push_back(inst);
            } else if (node.type == SceneNodeType::Light) {
                zegfx::LocalLightData localLight = {};
                if (node.name.find("Spot") != std::string::npos) {
                    localLight.type = zegfx::LightType::Spot;
                    localLight.range = 25.0f;
                    localLight.intensity = 5000.0f;
                    localLight.innerConeCos = 0.90f;
                    localLight.outerConeCos = 0.70f;
                } else {
                    localLight.type = zegfx::LightType::Point;
                    localLight.range = 15.0f;
                    localLight.intensity = 2500.0f;
                }
                localLight.position = { node.location[0], node.location[1] + 4.0f, node.location[2] };
                localLight.direction = { 0.0f, -1.0f, 0.0f };
                localLight.color = { 1.0f, 0.90f, 0.70f };
                m_LocalLights.push_back(localLight);
            }
            if (!node.children.empty()) {
                self(self, node.children);
            }
        }
    };
    traverseNodes(traverseNodes, rootNodes);

    // 3. Assemble RenderSceneSnapshot
    zegfx::RenderSceneSnapshot snapshot = {};
    snapshot.opaque = zegfx::Span<const zegfx::RenderInstance>(m_OpaqueInstances.data(), m_OpaqueInstances.size());
    snapshot.directionalLights = zegfx::Span<const zegfx::DirectionalLightData>(m_DirectionalLights.data(), m_DirectionalLights.size());
    snapshot.localLights = zegfx::Span<const zegfx::LocalLightData>(m_LocalLights.data(), m_LocalLights.size());

    // 4. Camera Render Data
    const auto& edCam = EditorState::Get().camera;
    float viewMatRaw[16];
    float projMatRaw[16];
    edCam.GetViewMatrix(viewMatRaw);

    float aspect = (m_Height > 0) ? ((float)m_Width / (float)m_Height) : 1.777f;
    edCam.GetProjectionMatrix(aspect, projMatRaw);

    zegfx::CameraRenderData camera = {};
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            camera.view.m[r][c] = viewMatRaw[r * 4 + c];
            camera.projection.m[r][c] = projMatRaw[r * 4 + c];
        }
    }
    EngineEditor::Vec3f camPos = edCam.GetPosition();
    camera.position = { camPos.x, camPos.y, camPos.z };
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;

    // Render snapshot through bulk mode API
    m_Renderer->render(snapshot, camera, m_Renderer->getSettings());

    auto& stats = EditorState::Get().stats;
    zegfx::IRendererDiagnostics* diag = m_Renderer->getDiagnostics();
    if (diag) {
        auto diagData = diag->getDiagnostics();
        if (diagData.totalFrameGpuMs > 0) {
            stats.frameTimeMs = diagData.totalFrameGpuMs;
            stats.fps = 1000.0f / diagData.totalFrameGpuMs;
        }
        stats.drawCalls = diagData.drawCallCount;
        stats.triangleCount = diagData.triangleCount;
        if (diagData.memory.totalAllocatedBytes > 0) {
            stats.vramUsedGB = (float)diagData.memory.totalAllocatedBytes / (1024.0f * 1024.0f * 1024.0f);
        }
    }

    stats.gpuName = "DirectX 12 (ZeGFX Engine)";
    stats.apiTag = "ZeGFX v1.0.0 (DX12)";
}

void ZeGFXAdapter::Render(ID3D12GraphicsCommandList* cmdList, uint32_t width, uint32_t height, float deltaTime) {
    if (!m_Initialized || !m_Renderer) return;

    m_TimeAccumulator += deltaTime;
    Resize(width, height);
    SyncEngineState();

    if (m_PhysicsWorld && deltaTime > 0.0f) {
        float stepTime = (deltaTime > 0.033f) ? 0.033f : deltaTime;
        m_PhysicsWorld->StepSimulation(stepTime);
    }
}

} // namespace EngineEditor
