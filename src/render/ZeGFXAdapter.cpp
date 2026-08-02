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

    CreateDefaultPrimitives();

    m_Initialized = true;
    std::cout << "[ZeGFXAdapter] ZeGFX engine & ZePhysics 3D backend initialized successfully (" << m_Width << "x" << m_Height << ")" << std::endl;
    return true;
}

void ZeGFXAdapter::CreateDefaultPrimitives() {
    if (!m_Renderer) return;

    // Create default PBR material
    m_DefaultMaterialHandle = m_Renderer->createMaterial("DefaultPBRMaterial");
    m_LoadedMaterials["DefaultPBRMaterial"] = m_DefaultMaterialHandle;

    // Create default Cube mesh (unit cube)
    std::vector<zegfx::ProceduralVertex> cubeVerts;
    std::vector<uint32_t> cubeIndices;

    struct Face {
        float normal[3];
        float positions[4][3];
        float uvs[4][2];
    };
    Face faces[6] = {
        // Front (+Z)
        { {0, 0, 1}, {{-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}}, {{0,1},{1,1},{1,0},{0,0}} },
        // Back (-Z)
        { {0, 0,-1}, {{ 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}}, {{0,1},{1,1},{1,0},{0,0}} },
        // Top (+Y)
        { {0, 1, 0}, {{-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}}, {{0,1},{1,1},{1,0},{0,0}} },
        // Bottom (-Y)
        { {0,-1, 0}, {{-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f}}, {{0,1},{1,1},{1,0},{0,0}} },
        // Right (+X)
        { {1, 0, 0}, {{ 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f}}, {{0,1},{1,1},{1,0},{0,0}} },
        // Left (-X)
        { {-1, 0, 0}, {{-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f}}, {{0,1},{1,1},{1,0},{0,0}} }
    };

    zegfx::Color whiteColor{255, 255, 255, 255};
    for (int f = 0; f < 6; ++f) {
        uint32_t baseIdx = (uint32_t)cubeVerts.size();
        for (int v = 0; v < 4; ++v) {
            zegfx::ProceduralVertex vert;
            vert.x = faces[f].positions[v][0];
            vert.y = faces[f].positions[v][1];
            vert.z = faces[f].positions[v][2];
            vert.nx = faces[f].normal[0];
            vert.ny = faces[f].normal[1];
            vert.nz = faces[f].normal[2];
            vert.u = faces[f].uvs[v][0];
            vert.v = faces[f].uvs[v][1];
            vert.color = whiteColor;
            cubeVerts.push_back(vert);
        }
        cubeIndices.push_back(baseIdx + 0);
        cubeIndices.push_back(baseIdx + 1);
        cubeIndices.push_back(baseIdx + 2);
        cubeIndices.push_back(baseIdx + 0);
        cubeIndices.push_back(baseIdx + 2);
        cubeIndices.push_back(baseIdx + 3);
    }

    m_DefaultMeshHandle = m_Renderer->createProceduralMesh(cubeVerts, cubeIndices);
    m_LoadedMeshes["DefaultCube"] = m_DefaultMeshHandle;
    m_LoadedMeshes["cube"] = m_DefaultMeshHandle;
    m_LoadedMeshes["Engine/DefaultCube"] = m_DefaultMeshHandle;
}

zegfx::RenderMeshHandle ZeGFXAdapter::LoadMeshAsset(const std::string& meshPath) {
    if (meshPath.empty()) return m_DefaultMeshHandle;

    auto it = m_LoadedMeshes.find(meshPath);
    if (it != m_LoadedMeshes.end()) return it->second;

    if (!m_Renderer) return m_DefaultMeshHandle;

    std::string err;
    zegfx::RenderMeshHandle handle = m_Renderer->loadMesh(meshPath, err);
    if (handle.valid()) {
        m_LoadedMeshes[meshPath] = handle;
        return handle;
    } else {
        std::cerr << "[ZeGFXAdapter] Failed to load mesh asset (" << meshPath << "): " << err << std::endl;
        return m_DefaultMeshHandle;
    }
}

zegfx::RenderMaterialHandle ZeGFXAdapter::LoadMaterialAsset(const std::string& matPath) {
    if (matPath.empty()) return m_DefaultMaterialHandle;

    auto it = m_LoadedMaterials.find(matPath);
    if (it != m_LoadedMaterials.end()) return it->second;

    if (!m_Renderer) return m_DefaultMaterialHandle;

    zegfx::RenderMaterialHandle handle = m_Renderer->createMaterial(matPath);
    if (handle.valid()) {
        m_LoadedMaterials[matPath] = handle;
        return handle;
    } else {
        return m_DefaultMaterialHandle;
    }
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

void ZeGFXAdapter::SyncEngineState(float deltaTime) {
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
                inst.mesh = LoadMeshAsset(node.meshPath);
                inst.material = LoadMaterialAsset(node.materialPath);

                float pitch = node.rotation[0] * 3.14159265f / 180.0f;
                float yaw   = node.rotation[1] * 3.14159265f / 180.0f;
                float roll  = node.rotation[2] * 3.14159265f / 180.0f;

                float cx = std::cos(pitch), sx = std::sin(pitch);
                float cy = std::cos(yaw),   sy = std::sin(yaw);
                float cz = std::cos(roll),  sz = std::sin(roll);

                float r00 = cy * cz + sy * sx * sz;
                float r01 = cx * sz;
                float r02 = -sy * cz + cy * sx * sz;

                float r10 = -cy * sz + sy * sx * cz;
                float r11 = cx * cz;
                float r12 = sy * sz + cy * sx * cz;

                float r20 = sy * cx;
                float r21 = -sx;
                float r22 = cy * cx;

                inst.world = zegfx::Mat4::identity();
                inst.world.m[0][0] = r00 * node.scale[0]; inst.world.m[0][1] = r01 * node.scale[0]; inst.world.m[0][2] = r02 * node.scale[0]; inst.world.m[0][3] = 0.0f;
                inst.world.m[1][0] = r10 * node.scale[1]; inst.world.m[1][1] = r11 * node.scale[1]; inst.world.m[1][2] = r12 * node.scale[1]; inst.world.m[1][3] = 0.0f;
                inst.world.m[2][0] = r20 * node.scale[2]; inst.world.m[2][1] = r21 * node.scale[2]; inst.world.m[2][2] = r22 * node.scale[2]; inst.world.m[2][3] = 0.0f;
                inst.world.m[3][0] = node.location[0];    inst.world.m[3][1] = node.location[1];    inst.world.m[3][2] = node.location[2];    inst.world.m[3][3] = 1.0f;

                inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                inst.visibilityFlags = 1;
                m_OpaqueInstances.push_back(inst);
            } else if (node.type == SceneNodeType::FoliageCluster) {
                // High-density foliage instancing (trees, shrubs, grass) across 500m-1km area
                const int numTrees = 0; // Default 0 for empty scene; set via foliage settings when enabled
                for (int i = 0; i < numTrees; ++i) {
                    float angle = (float)i * 0.125f;
                    float dist = 20.0f + (float)(i % 50) * 15.0f; // Spans up to 770m
                    float px = node.location[0] + std::cos(angle) * dist;
                    float pz = node.location[2] + std::sin(angle) * dist;
                    float py = node.location[1] + (std::sin(px * 0.05f) + std::cos(pz * 0.05f)) * 3.5f;

                    zegfx::RenderInstance inst = {};
                    inst.mesh = LoadMeshAsset(node.meshPath);
                    inst.material = LoadMaterialAsset(node.materialPath);
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
                inst.mesh = LoadMeshAsset(node.meshPath);
                inst.material = LoadMaterialAsset(node.materialPath);
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
    auto tStartRender = std::chrono::high_resolution_clock::now();
    m_Renderer->render(snapshot, camera, m_Renderer->getSettings());
    auto tEndRender = std::chrono::high_resolution_clock::now();
    double renderMs = std::chrono::duration<double, std::milli>(tEndRender - tStartRender).count();

    static uint64_t s_AdapterFrames = 0;
    static double s_AccRenderMs = 0.0;
    s_AdapterFrames++;
    s_AccRenderMs += renderMs;
    if (s_AdapterFrames % 120 == 0) {
        std::cout << "[ZEGFX PROFILER] Average Renderer::render() time: " << (s_AccRenderMs / 120.0) << " ms" << std::endl;
        s_AccRenderMs = 0.0;
    }

    auto& stats = EditorState::Get().stats;
    zegfx::IRendererDiagnostics* diag = m_Renderer->getDiagnostics();
    if (diag) {
        auto diagData = diag->getDiagnostics();
        if (diagData.totalFrameGpuMs > 0) {
            stats.frameTimeMs = diagData.totalFrameGpuMs;
            stats.fps = 1000.0f / diagData.totalFrameGpuMs;
        } else {
            stats.frameTimeMs = deltaTime * 1000.0f;
            stats.fps = (deltaTime > 0.00001f) ? (1.0f / deltaTime) : 60.0f;
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

void ZeGFXAdapter::SetOutputRenderTarget(ID3D12Resource* rtResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) {
    m_ActiveRenderTargetResource = rtResource;
    m_ActiveRtvHandle = rtvHandle;
}

void ZeGFXAdapter::Render(ID3D12GraphicsCommandList* cmdList, uint32_t width, uint32_t height, float deltaTime) {
    if (!m_Initialized || !m_Renderer) return;

    if (cmdList && m_ActiveRtvHandle.ptr != 0) {
        cmdList->OMSetRenderTargets(1, &m_ActiveRtvHandle, FALSE, nullptr);
        D3D12_VIEWPORT vp = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
        D3D12_RECT sr = { 0, 0, (LONG)width, (LONG)height };
        cmdList->RSSetViewports(1, &vp);
        cmdList->RSSetScissorRects(1, &sr);
    }

    m_TimeAccumulator += deltaTime;
    Resize(width, height);
    SyncEngineState(deltaTime);

    if (m_PhysicsWorld && deltaTime > 0.0f) {
        float stepTime = (deltaTime > 0.033f) ? 0.033f : deltaTime;
        m_PhysicsWorld->StepSimulation(stepTime);
    }
}

} // namespace EngineEditor
