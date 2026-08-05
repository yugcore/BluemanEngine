#include "ZeGFXAdapter.h"
#include "zegfx.h"
#include "cooker/asset_cooker.h"
#include "core/EditorState.h"
#include "core/ComponentRegistry.h"
#include "engine/scene/SceneGraph.h"
#include "engine/scene/SunOrbitController.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <filesystem>
#include <algorithm>

#include "physics/physics_world.h"

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#endif

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

    zegfx::RendererSettings settings = zegfx::RendererSettings::fromPreset(zegfx::QualityPreset::Ultra);
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

    // Configure initial directional sun light (exact Zenbo setup)
    zegfx::DirectionalLightData sunLight = {};
    float sDir[3] = { -0.5f, -0.8f, -0.3f };
    float sLen = std::sqrt(sDir[0]*sDir[0] + sDir[1]*sDir[1] + sDir[2]*sDir[2]);
    if (sLen > 0.0001f) { sDir[0] /= sLen; sDir[1] /= sLen; sDir[2] /= sLen; }
    sunLight.direction = { sDir[0], sDir[1], sDir[2] };
    sunLight.color = { 1.0f, 0.95f, 0.85f };
    sunLight.illuminanceLux = 100000.0f;
    sunLight.angularDiameterRadians = 0.0093f;
    m_Renderer->setDirectionalLight(sunLight);

    // Phase 5: Register Sky/Atmosphere Observer (Decoupled Event, No Hard Reference)
    SunOrbitController::Get().RegisterSunChangeListener([this](const float dir[3], float elevationDeg) {
        if (!m_HasSkyAtmosphere) {
            m_SkyAmbientColor = zegfx::Color(0, 0, 0, 255);
            return;
        }
        float elevNorm = std::max(0.0f, std::min(1.0f, (elevationDeg + 10.0f) / 100.0f));
        if (elevationDeg > 0.0f) {
            uint8_t r = static_cast<uint8_t>(51.0f * elevNorm + 245.0f * (1.0f - elevNorm));
            uint8_t g = static_cast<uint8_t>(107.0f * elevNorm + 160.0f * (1.0f - elevNorm));
            uint8_t b = static_cast<uint8_t>(191.0f * elevNorm + 100.0f * (1.0f - elevNorm));
            m_SkyAmbientColor = zegfx::Color(r, g, b, 255);
        } else {
            m_SkyAmbientColor = zegfx::Color(10, 15, 30, 255);
        }
    });

    CreateDefaultPrimitives();

    m_Initialized = true;
    std::cout << "[ZeGFXAdapter] ZeGFX engine & ZePhysics 3D backend initialized successfully (" << m_Width << "x" << m_Height << ")" << std::endl;
    return true;
}

void ZeGFXAdapter::CreateDefaultPrimitives() {
    if (!m_Renderer) return;

    // Create default PBR material
    m_DefaultMaterialHandle = m_Renderer->createMaterial("DefaultPBRMaterial");
    m_Renderer->setMaterialColor(m_DefaultMaterialHandle, "baseColor", zegfx::Color(200, 200, 200, 255));
    m_Renderer->setMaterialFloat(m_DefaultMaterialHandle, "roughness", 0.5f);
    m_Renderer->setMaterialFloat(m_DefaultMaterialHandle, "metallic", 0.0f);
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
    m_LoadedMeshes["primitives/cube.zmesh"] = m_DefaultMeshHandle;

    // Create default Ground Plane mesh (100m x 100m at Y=0)
    std::vector<zegfx::ProceduralVertex> planeVerts;
    std::vector<uint32_t> planeIndices;
    float halfSize = 50.0f;
    float uvTile = 10.0f;
    zegfx::ProceduralVertex p0 = { -halfSize, 0.0f,  halfSize, 0.0f, 1.0f, 0.0f, 0.0f,   0.0f,   whiteColor };
    zegfx::ProceduralVertex p1 = {  halfSize, 0.0f,  halfSize, 0.0f, 1.0f, 0.0f, uvTile, 0.0f,   whiteColor };
    zegfx::ProceduralVertex p2 = {  halfSize, 0.0f, -halfSize, 0.0f, 1.0f, 0.0f, uvTile, uvTile, whiteColor };
    zegfx::ProceduralVertex p3 = { -halfSize, 0.0f, -halfSize, 0.0f, 1.0f, 0.0f, 0.0f,   uvTile, whiteColor };
    planeVerts.push_back(p0); planeVerts.push_back(p1); planeVerts.push_back(p2); planeVerts.push_back(p3);
    planeIndices = { 0, 1, 2, 0, 2, 3 };

    m_DefaultPlaneMeshHandle = m_Renderer->createProceduralMesh(planeVerts, planeIndices);
    m_LoadedMeshes["DefaultPlane"] = m_DefaultPlaneMeshHandle;
    m_LoadedMeshes["plane"] = m_DefaultPlaneMeshHandle;
    m_LoadedMeshes["Engine/DefaultPlane"] = m_DefaultPlaneMeshHandle;
    m_LoadedMeshes["primitives/plane.zmesh"] = m_DefaultPlaneMeshHandle;

    // Create default Sphere mesh (UV Sphere, r=0.5)
    std::vector<zegfx::ProceduralVertex> sphereVerts;
    std::vector<uint32_t> sphereIndices;
    const int sphereStacks = 16;
    const int sphereSlices = 16;
    const float radius = 0.5f;
    const float PI = 3.1415926535f;

    for (int i = 0; i <= sphereStacks; ++i) {
        float V = (float)i / (float)sphereStacks;
        float phi = V * PI;
        for (int j = 0; j <= sphereSlices; ++j) {
            float U = (float)j / (float)sphereSlices;
            float theta = U * (2.0f * PI);

            float x = std::cos(theta) * std::sin(phi);
            float y = std::cos(phi);
            float z = std::sin(theta) * std::sin(phi);

            zegfx::ProceduralVertex v;
            v.x = x * radius; v.y = y * radius; v.z = z * radius;
            v.nx = x; v.ny = y; v.nz = z;
            v.u = U; v.v = V;
            v.color = whiteColor;
            sphereVerts.push_back(v);
        }
    }
    for (int i = 0; i < sphereStacks; ++i) {
        for (int j = 0; j < sphereSlices; ++j) {
            uint32_t first = (i * (sphereSlices + 1)) + j;
            uint32_t second = first + sphereSlices + 1;
            sphereIndices.push_back(first);
            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);

            sphereIndices.push_back(second);
            sphereIndices.push_back(second + 1);
            sphereIndices.push_back(first + 1);
        }
    }
    m_DefaultSphereMeshHandle = m_Renderer->createProceduralMesh(sphereVerts, sphereIndices);
    m_LoadedMeshes["DefaultSphere"] = m_DefaultSphereMeshHandle;
    m_LoadedMeshes["sphere"] = m_DefaultSphereMeshHandle;
    m_LoadedMeshes["Engine/DefaultSphere"] = m_DefaultSphereMeshHandle;
    m_LoadedMeshes["primitives/sphere.zmesh"] = m_DefaultSphereMeshHandle;

    // Create default Cylinder mesh (r=0.5, h=1.0)
    std::vector<zegfx::ProceduralVertex> cylVerts;
    std::vector<uint32_t> cylIndices;
    const int cylSlices = 16;
    for (int i = 0; i <= cylSlices; ++i) {
        float u = (float)i / (float)cylSlices;
        float theta = u * 2.0f * PI;
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);

        zegfx::ProceduralVertex vTop = { cosT * 0.5f,  0.5f, sinT * 0.5f, cosT, 0.0f, sinT, u, 0.0f, whiteColor };
        zegfx::ProceduralVertex vBot = { cosT * 0.5f, -0.5f, sinT * 0.5f, cosT, 0.0f, sinT, u, 1.0f, whiteColor };
        cylVerts.push_back(vTop);
        cylVerts.push_back(vBot);
    }
    for (int i = 0; i < cylSlices; ++i) {
        uint32_t idx = i * 2;
        cylIndices.push_back(idx);
        cylIndices.push_back(idx + 1);
        cylIndices.push_back(idx + 2);
        cylIndices.push_back(idx + 1);
        cylIndices.push_back(idx + 3);
        cylIndices.push_back(idx + 2);
    }
    m_DefaultCylinderMeshHandle = m_Renderer->createProceduralMesh(cylVerts, cylIndices);
    m_LoadedMeshes["DefaultCylinder"] = m_DefaultCylinderMeshHandle;
    m_LoadedMeshes["cylinder"] = m_DefaultCylinderMeshHandle;
    m_LoadedMeshes["Engine/DefaultCylinder"] = m_DefaultCylinderMeshHandle;
    m_LoadedMeshes["primitives/cylinder.zmesh"] = m_DefaultCylinderMeshHandle;

    // Create default Cone mesh (r=0.5, h=1.0)
    std::vector<zegfx::ProceduralVertex> coneVerts;
    std::vector<uint32_t> coneIndices;
    const int coneSlices = 16;
    zegfx::ProceduralVertex apex = { 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, whiteColor };
    coneVerts.push_back(apex);
    for (int i = 0; i <= coneSlices; ++i) {
        float u = (float)i / (float)coneSlices;
        float theta = u * 2.0f * PI;
        float cosT = std::cos(theta);
        float sinT = std::sin(theta);
        zegfx::ProceduralVertex v = { cosT * 0.5f, -0.5f, sinT * 0.5f, cosT, 0.5f, sinT, u, 1.0f, whiteColor };
        coneVerts.push_back(v);
    }
    for (int i = 1; i <= coneSlices; ++i) {
        coneIndices.push_back(0);
        coneIndices.push_back(i);
        coneIndices.push_back(i + 1);
    }
    m_DefaultConeMeshHandle = m_Renderer->createProceduralMesh(coneVerts, coneIndices);
    m_LoadedMeshes["DefaultCone"] = m_DefaultConeMeshHandle;
    m_LoadedMeshes["cone"] = m_DefaultConeMeshHandle;
    m_LoadedMeshes["Engine/DefaultCone"] = m_DefaultConeMeshHandle;
    m_LoadedMeshes["primitives/cone.zmesh"] = m_DefaultConeMeshHandle;

}

void ZeGFXAdapter::SetLightingDebugMode(int mode) {
    EditorState::Get().settings.lightingDebugMode = mode;
    if (m_Renderer) {
        m_Renderer->setLightingDebugMode((zegfx::LightingDebugMode)mode);
    }
}

zegfx::RenderMeshHandle ZeGFXAdapter::LoadMeshAsset(const std::string& meshPath) {
    if (meshPath.empty()) return m_DefaultMeshHandle;
    
    // Check map cache first
    auto it = m_LoadedMeshes.find(meshPath);
    if (it != m_LoadedMeshes.end()) return it->second;

    if (!m_Renderer) return m_DefaultMeshHandle;

    namespace fs = std::filesystem;
    std::string cookPath = meshPath;
    std::string ext = fs::path(meshPath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // If raw 3D mesh format, cook it to .zmesh first using AssetCooker if not already cooked
    if (ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".fbx" || ext == ".vox") {
        std::string projectDir = "CookedAssets";
        std::error_code ec;
        fs::create_directories(projectDir, ec);
        std::string stem = fs::path(meshPath).stem().string();
        std::string targetOut = projectDir + "/" + stem + ".zmesh";

        if (fs::exists(targetOut)) {
            cookPath = targetOut;
        } else {
            zegfx::cooker::AssetCooker cooker;
            std::cout << "[ZeGFXAdapter] Cooking raw asset '" << meshPath << "' to '" << targetOut << "'..." << std::endl;
            if (cooker.CookMesh(meshPath, targetOut)) {
                cookPath = targetOut;
            } else {
                std::cerr << "[ZeGFXAdapter] AssetCooker fallback for '" << meshPath << "'" << std::endl;
            }
        }
    }

    std::string err;
    zegfx::RenderMeshHandle handle = m_Renderer->loadMesh(cookPath, err);
    if (handle.valid()) {
        m_LoadedMeshes[meshPath] = handle;
        m_LoadedMeshes[cookPath] = handle;
        return handle;
    } else {
        std::cerr << "[ZeGFXAdapter] Failed to load mesh asset (" << cookPath << "): " << err << std::endl;
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

std::string ZeGFXAdapter::CreateTerrainFromHeightmap(const std::string& name, const std::string& filePath, const zegfx::HeightmapImportSettings& settings, std::string& outError) {
    std::ofstream dbg("heightmap_import_crash_debug.log", std::ios::app);
    if (dbg.is_open()) {
        dbg << "[ZeGFXAdapter] CreateTerrainFromHeightmap START name=" << name << " file=" << filePath
            << " targetW=" << settings.targetWidth << " targetH=" << settings.targetHeight << std::endl;
    }

    if (!m_Renderer) {
        outError = "Renderer not initialized.";
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] FAIL: Renderer not initialized" << std::endl;
        return "";
    }

    zegfx::HeightmapImportResult importRes;
    try {
        importRes = zegfx::HeightmapImporter::LoadFromFile(filePath, settings);
    } catch (const std::exception& e) {
        outError = std::string("Exception in HeightmapImporter::LoadFromFile: ") + e.what();
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] CRASH EXCEPTION in LoadFromFile: " << e.what() << std::endl;
        return "";
    } catch (...) {
        outError = "Unknown exception in HeightmapImporter::LoadFromFile";
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] CRASH UNKNOWN EXCEPTION in LoadFromFile" << std::endl;
        return "";
    }

    if (!importRes.success) {
        outError = importRes.errorMessage;
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] FAIL LoadFromFile: " << importRes.errorMessage << std::endl;
        return "";
    }

    int W = importRes.targetWidth;
    int H = importRes.targetHeight;
    float cellSize = settings.cellSize;
    float heightScale = settings.heightScale;

    if (dbg.is_open()) {
        dbg << "[ZeGFXAdapter] LoadFromFile success. W=" << W << " H=" << H
            << " heights.size=" << importRes.heights.size() << std::endl;
    }

    if (importRes.heights.size() < static_cast<size_t>(W * H)) {
        outError = "Heightmap data size mismatch: heights.size < W*H";
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] FAIL: heights.size (" << importRes.heights.size() << ") < W*H (" << (W*H) << ")" << std::endl;
        return "";
    }

    float halfSizeX = (W - 1) * cellSize * 0.5f;
    float halfSizeZ = (H - 1) * cellSize * 0.5f;

    std::vector<zegfx::ProceduralVertex> vertices;
    std::vector<uint32_t> indices;

    try {
        vertices.reserve(W * H);
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] Building vertices W=" << W << " H=" << H << std::endl;

        for (int r = 0; r < H; ++r) {
            float localZ = r * cellSize - halfSizeZ;
            for (int c = 0; c < W; ++c) {
                float localX = c * cellSize - halfSizeX;
                float y = importRes.heights[r * W + c] * heightScale;

                // Finite difference normals
                float eps = cellSize;
                float hL = importRes.heights[r * W + std::max(0, c - 1)] * heightScale;
                float hR = importRes.heights[r * W + std::min(W - 1, c + 1)] * heightScale;
                float hD = importRes.heights[std::max(0, r - 1) * W + c] * heightScale;
                float hU = importRes.heights[std::min(H - 1, r + 1) * W + c] * heightScale;

                float nvx = hL - hR;
                float nvy = 2.0f * eps;
                float nvz = hD - hU;
                float len = std::sqrt(nvx * nvx + nvy * nvy + nvz * nvz);
                float nx = (len > 0.0f) ? nvx / len : 0.0f;
                float ny = (len > 0.0f) ? nvy / len : 1.0f;
                float nz = (len > 0.0f) ? nvz / len : 0.0f;

                zegfx::ProceduralVertex v = {};
                v.x = localX;
                v.y = y;
                v.z = localZ;
                v.nx = nx;
                v.ny = ny;
                v.nz = nz;
                v.u = (float)c / (W - 1);
                v.v = (float)r / (H - 1);

                // Solid white terrain as requested by user
                v.color = zegfx::Color(255, 255, 255, 255);

                vertices.push_back(v);
            }
        }

        if (dbg.is_open()) dbg << "[ZeGFXAdapter] Building indices..." << std::endl;
        indices.reserve((W - 1) * (H - 1) * 6);
        for (int r = 0; r < H - 1; ++r) {
            for (int c = 0; c < W - 1; ++c) {
                uint32_t i0 = r * W + c;
                uint32_t i1 = r * W + (c + 1);
                uint32_t i2 = (r + 1) * W + c;
                uint32_t i3 = (r + 1) * W + (c + 1);

                // Front-facing (upward normal +Y) winding order
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);

                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }
    } catch (const std::exception& e) {
        outError = std::string("Exception building terrain mesh vectors: ") + e.what();
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] CRASH EXCEPTION building mesh: " << e.what() << std::endl;
        return "";
    }

    if (dbg.is_open()) dbg << "[ZeGFXAdapter] Calling createProceduralMesh..." << std::endl;

    zegfx::RenderMeshHandle handle;
    try {
        handle = m_Renderer->createProceduralMesh(vertices, indices);
    } catch (const std::exception& e) {
        outError = std::string("Exception in createProceduralMesh: ") + e.what();
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] CRASH EXCEPTION in createProceduralMesh: " << e.what() << std::endl;
        return "";
    }

    if (!handle.valid()) {
        outError = "Failed to create procedural mesh on GPU renderer.";
        if (dbg.is_open()) dbg << "[ZeGFXAdapter] FAIL: createProceduralMesh returned invalid handle" << std::endl;
        return "";
    }

    std::string meshKey = "TerrainMesh_" + name;
    m_LoadedMeshes[meshKey] = handle;

    if (dbg.is_open()) {
        dbg << "[ZeGFXAdapter] SUCCESS Created terrain mesh key: " << meshKey
            << " vertices=" << vertices.size() << " indices=" << indices.size() << std::endl;
    }

    return meshKey;
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

static void QuerySystemPerformanceStats(float& outRamUsedGB, float& outVramUsedGB, float& outVramTotalGB, std::string& outGpuName) {
#ifdef _WIN32
    // 1. Process RAM Usage (Working Set Size)
    PROCESS_MEMORY_COUNTERS pmc = {};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        outRamUsedGB = (float)pmc.WorkingSetSize / (1024.0f * 1024.0f * 1024.0f);
    }

    // 2. DXGI VRAM & GPU Device Name Query
    static bool s_GpuNameQueried = false;
    static std::string s_CachedGpuName = "DirectX 12 GPU";

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (SUCCEEDED(factory->EnumAdapters1(0, &adapter))) {
            if (!s_GpuNameQueried) {
                DXGI_ADAPTER_DESC1 desc = {};
                if (SUCCEEDED(adapter->GetDesc1(&desc))) {
                    char nameBuf[128] = {};
                    wcstombs(nameBuf, desc.Description, sizeof(nameBuf) - 1);
                    if (nameBuf[0] != '\0') s_CachedGpuName = nameBuf;
                }
                s_GpuNameQueried = true;
            }

            Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter3;
            if (SUCCEEDED(adapter.As(&adapter3))) {
                DXGI_QUERY_VIDEO_MEMORY_INFO info = {};
                if (SUCCEEDED(adapter3->QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &info))) {
                    outVramUsedGB = (float)info.CurrentUsage / (1024.0f * 1024.0f * 1024.0f);
                    outVramTotalGB = (float)info.Budget / (1024.0f * 1024.0f * 1024.0f);
                }
            }
        }
    }
    outGpuName = s_CachedGpuName;
#endif
}

void ZeGFXAdapter::SyncEngineState(float deltaTime, zegfx::ExternalCmdListHandle externalCmdList) {
    if (!m_Renderer) return;

    const auto& edSettings = EditorState::Get().settings;

    // Set offscreen RTV target on ZeGFX backend
    if (m_ActiveRtvHandle.ptr != 0) {
        m_Renderer->setExternalRenderTargetCPU(m_ActiveRtvHandle.ptr);
    }

    // Set lighting debug mode
    m_Renderer->setLightingDebugMode((zegfx::LightingDebugMode)edSettings.lightingDebugMode);

    static bool s_FirstSync = true;
    static uint32_t s_LastWidth = 0, s_LastHeight = 0;
    static RenderSettings s_LastSettings = {};

    bool rtAO = (edSettings.ao.mode == 2) || edSettings.rtAO;
    bool rtRefl = edSettings.rtReflections;
    bool rtGI = edSettings.rtGI;
    bool fog = edSettings.fog.enableVolumetric;

    bool settingsChanged = s_FirstSync ||
                           (s_LastWidth != m_Width) ||
                           (s_LastHeight != m_Height) ||
                           (s_LastSettings.qualityPreset != edSettings.qualityPreset) ||
                           (s_LastSettings.rtAO != edSettings.rtAO) ||
                           (s_LastSettings.rtReflections != edSettings.rtReflections) ||
                           (s_LastSettings.rtGI != edSettings.rtGI) ||
                           (s_LastSettings.giRaysPerProbe != edSettings.giRaysPerProbe) ||
                           (s_LastSettings.giProbesUpdatedPerFrame != edSettings.giProbesUpdatedPerFrame) ||
                           (s_LastSettings.fog.enableVolumetric != edSettings.fog.enableVolumetric) ||
                           (s_LastSettings.fog.density != edSettings.fog.density) ||
                           (s_LastSettings.fog.color[0] != edSettings.fog.color[0]) ||
                           (s_LastSettings.fog.color[1] != edSettings.fog.color[1]) ||
                           (s_LastSettings.fog.color[2] != edSettings.fog.color[2]) ||
                           (s_LastSettings.shadow.cascadeResolution != edSettings.shadow.cascadeResolution) ||
                           (s_LastSettings.shadow.cascadeCount != edSettings.shadow.cascadeCount) ||
                           (s_LastSettings.shadow.maxDistance != edSettings.shadow.maxDistance) ||
                           (s_LastSettings.shadow.constantBias != edSettings.shadow.constantBias) ||
                           (s_LastSettings.shadow.slopeBias != edSettings.shadow.slopeBias) ||
                           (s_LastSettings.shadow.normalBias != edSettings.shadow.normalBias) ||
                           (s_LastSettings.shadow.filterSoftness != edSettings.shadow.filterSoftness) ||
                           (s_LastSettings.ao.mode != edSettings.ao.mode) ||
                           (s_LastSettings.ao.radius != edSettings.ao.radius) ||
                           (s_LastSettings.ao.intensity != edSettings.ao.intensity) ||
                           (s_LastSettings.ao.temporalFiltering != edSettings.ao.temporalFiltering) ||
                           (s_LastSettings.postFX.autoExposure != edSettings.postFX.autoExposure) ||
                           (s_LastSettings.postFX.exposureEV != edSettings.postFX.exposureEV) ||
                           (s_LastSettings.postFX.bloomIntensity != edSettings.postFX.bloomIntensity) ||
                           (s_LastSettings.postFX.bloomThreshold != edSettings.postFX.bloomThreshold);

    if (settingsChanged) {
        s_FirstSync = false;
        s_LastWidth = m_Width;
        s_LastHeight = m_Height;
        s_LastSettings = edSettings;

        int preset = edSettings.qualityPreset;
        zegfx::QualityPreset qp = zegfx::QualityPreset::Ultra;
        if (preset == 0) qp = zegfx::QualityPreset::Low;
        else if (preset == 1) qp = zegfx::QualityPreset::Medium;
        else if (preset == 2) qp = zegfx::QualityPreset::High;
        else qp = zegfx::QualityPreset::Ultra;

        zegfx::RendererSettings rSettings = zegfx::RendererSettings::fromPreset(qp);
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
        rSettings.fog.density = (edSettings.fog.density > 0.0001f) ? edSettings.fog.density : 0.005f;
        rSettings.fog.heightFalloff = 0.08f;
        rSettings.fog.cameraHeight = EditorState::Get().camera.GetPosition().y;
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

    // 1. Traversal of SceneGraph Nodes for Directional Lights, Mesh Entities, Foliage, Terrain, Sky & Local Lights
    const auto& rootNodes = SceneGraph::Get().GetRootNodes();

    bool skyAtmosphereFound = false;
    SkyAtmosphereProxy activeSkyProxy = {};

    bool volumetricFogFound = false;
    VolumetricFogProxy activeFogProxy = {};

    auto traverseNodes = [&](auto& self, const std::vector<SceneNode>& nodes) -> void {
        for (const auto& node : nodes) {
            if (!node.visible) {
                continue;
            }

            const TransformComponent* transformComp = ComponentRegistry::Get().GetComponent<TransformComponent>(node.id);
            const MeshComponent* meshComp = ComponentRegistry::Get().GetComponent<MeshComponent>(node.id);
            const MaterialComponent* matComp = ComponentRegistry::Get().GetComponent<MaterialComponent>(node.id);
            const LightComponent* lightComp = ComponentRegistry::Get().GetComponent<LightComponent>(node.id);

            float loc[3] = { node.location[0], node.location[1], node.location[2] };
            float rot[3] = { node.rotation[0], node.rotation[1], node.rotation[2] };
            float scl[3] = { node.scale[0], node.scale[1], node.scale[2] };
            std::string meshPath = node.meshPath;
            std::string matPath = node.materialPath;

            if (transformComp) {
                loc[0] = transformComp->location[0]; loc[1] = transformComp->location[1]; loc[2] = transformComp->location[2];
                rot[0] = transformComp->rotation[0]; rot[1] = transformComp->rotation[1]; rot[2] = transformComp->rotation[2];
                scl[0] = transformComp->scale[0];    scl[1] = transformComp->scale[1];    scl[2] = transformComp->scale[2];
            }
            if (meshComp && !meshComp->meshPath.empty()) {
                meshPath = meshComp->meshPath;
            }
            if (matComp && !matComp->materialPath.empty()) {
                matPath = matComp->materialPath;
            }

            if (node.type == SceneNodeType::FoliageCluster) {
                // High-density foliage instancing (trees, shrubs, grass) across 500m-1km area
                const int numTrees = node.treeCount > 0 ? node.treeCount : 100;
                std::string foliageMesh = meshPath.empty() ? "Engine/DefaultCone" : meshPath;
                std::string foliageMat = matPath.empty() ? "DefaultPBRMaterial" : matPath;
                for (int i = 0; i < numTrees; ++i) {
                    float angle = (float)i * 0.125f;
                    float dist = 20.0f + (float)(i % 50) * 15.0f; // Spans up to 770m
                    float px = loc[0] + std::cos(angle) * dist;
                    float pz = loc[2] + std::sin(angle) * dist;
                    float py = loc[1] + (std::sin(px * 0.05f) + std::cos(pz * 0.05f)) * 3.5f;

                    zegfx::RenderInstance inst = {};
                    inst.mesh = LoadMeshAsset(foliageMesh);
                    inst.material = LoadMaterialAsset(foliageMat);
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
                std::string pathMesh = meshPath.empty() ? "Engine/DefaultPlane" : meshPath;
                std::string pathMat = matPath.empty() ? "DefaultPBRMaterial" : matPath;
                zegfx::RenderInstance inst = {};
                inst.mesh = LoadMeshAsset(pathMesh);
                inst.material = LoadMaterialAsset(pathMat);
                inst.world = zegfx::Mat4::identity();
                inst.world.m[0][0] = 0.5f; inst.world.m[1][1] = 0.1f; inst.world.m[2][2] = 0.5f; // Flat path stone
                inst.world.m[3][0] = loc[0];
                inst.world.m[3][1] = loc[1];
                inst.world.m[3][2] = loc[2];
                inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                inst.visibilityFlags = 1;
                m_OpaqueInstances.push_back(inst);
            } else if (node.type == SceneNodeType::Actor || node.type == SceneNodeType::Terrain || meshComp != nullptr) {
                if (!meshPath.empty()) {
                    zegfx::RenderInstance inst = {};
                    inst.mesh = LoadMeshAsset(meshPath);
                    inst.material = LoadMaterialAsset(matPath);

                    float pitch = rot[0] * 3.14159265f / 180.0f;
                    float yaw   = rot[1] * 3.14159265f / 180.0f;
                    float roll  = rot[2] * 3.14159265f / 180.0f;

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
                    inst.world.m[0][0] = r00 * scl[0]; inst.world.m[0][1] = r01 * scl[0]; inst.world.m[0][2] = r02 * scl[0]; inst.world.m[0][3] = 0.0f;
                    inst.world.m[1][0] = r10 * scl[1]; inst.world.m[1][1] = r11 * scl[1]; inst.world.m[1][2] = r12 * scl[1]; inst.world.m[1][3] = 0.0f;
                    inst.world.m[2][0] = r20 * scl[2]; inst.world.m[2][1] = r21 * scl[2]; inst.world.m[2][2] = r22 * scl[2]; inst.world.m[2][3] = 0.0f;
                    inst.world.m[3][0] = loc[0];       inst.world.m[3][1] = loc[1];       inst.world.m[3][2] = loc[2];       inst.world.m[3][3] = 1.0f;

                    inst.objectId = (uint32_t)m_OpaqueInstances.size() + 1;
                    inst.visibilityFlags = 1;
                    m_OpaqueInstances.push_back(inst);
                }
            }

            // Process light components independently for any node
            DirectionalLightComponent* dirLightComp = ComponentRegistry::Get().GetComponent<DirectionalLightComponent>(node.id);
            TransformComponent* transCompMutable = ComponentRegistry::Get().GetComponent<TransformComponent>(node.id);

            if (node.type == SceneNodeType::Light || lightComp != nullptr || dirLightComp != nullptr) {
                int lightType = lightComp ? lightComp->lightType : 0; // 0: Directional, 1: Point, 2: Spot
                if (lightType == 0) {
                    // Drive Time-of-Day orbit rotation via OrbitController (Phase 3)
                    SunOrbitController::Get().Tick(deltaTime, dirLightComp, transCompMutable);

                    // Re-fetch updated rotation from transform component
                    if (transCompMutable) {
                        rot[0] = transCompMutable->rotation[0];
                        rot[1] = transCompMutable->rotation[1];
                        rot[2] = transCompMutable->rotation[2];
                    }

                    // Phase 4: Decoupled Render Proxy Extraction (Component -> Proxy POD -> GPU)
                    DirectionalLightProxy proxy = {};
                    DirectionalLightComponent::ComputeDirectionFromRotation(rot, proxy.direction);

                    if (dirLightComp) {
                        proxy.color[0] = dirLightComp->color[0];
                        proxy.color[1] = dirLightComp->color[1];
                        proxy.color[2] = dirLightComp->color[2];
                        proxy.illuminanceLux = dirLightComp->illuminanceLux;
                        proxy.angularDiameterRadians = dirLightComp->angularDiameterRadians;
                        proxy.castShadows = dirLightComp->castShadows;
                        proxy.cascadeCount = dirLightComp->cascadeCount;
                        proxy.shadowDistance = dirLightComp->shadowDistance;
                        proxy.cascadeDistributionExponent = dirLightComp->cascadeDistributionExponent;
                    } else if (lightComp) {
                        proxy.color[0] = lightComp->color[0];
                        proxy.color[1] = lightComp->color[1];
                        proxy.color[2] = lightComp->color[2];
                        proxy.illuminanceLux = (lightComp->intensity > 0.0f) ? lightComp->intensity : 100000.0f;
                        proxy.castShadows = lightComp->castShadows;
                    } else {
                        proxy.color[0] = 1.0f; proxy.color[1] = 0.95f; proxy.color[2] = 0.85f;
                        proxy.illuminanceLux = 100000.0f;
                        proxy.castShadows = true;
                    }
                    proxy.flags = proxy.castShadows ? 1u : 0u;

                    // Convert proxy to renderer snapshot format
                    zegfx::DirectionalLightData sunLight = {};
                    sunLight.direction = { proxy.direction[0], proxy.direction[1], proxy.direction[2] };
                    sunLight.color = { proxy.color[0], proxy.color[1], proxy.color[2] };
                    sunLight.illuminanceLux = proxy.illuminanceLux;
                    sunLight.angularDiameterRadians = proxy.angularDiameterRadians;
                    sunLight.flags = proxy.flags;

                    m_DirectionalLights.push_back(sunLight);
                } else {
                    zegfx::LocalLightData localLight = {};
                    if (lightComp) {
                        localLight.type = (lightType == 2) ? zegfx::LightType::Spot : zegfx::LightType::Point;
                        localLight.range = lightComp->range;
                        localLight.intensity = lightComp->intensity;
                        localLight.innerConeCos = std::cos(lightComp->innerCone * 3.14159265f / 180.0f);
                        localLight.outerConeCos = std::cos(lightComp->outerCone * 3.14159265f / 180.0f);
                        localLight.color = { lightComp->color[0], lightComp->color[1], lightComp->color[2] };
                    } else {
                        localLight.type = (lightType == 2) ? zegfx::LightType::Spot : zegfx::LightType::Point;
                        localLight.range = (lightType == 2) ? 25.0f : 15.0f;
                        localLight.intensity = (lightType == 2) ? 5000.0f : 2500.0f;
                        localLight.innerConeCos = 0.90f;
                        localLight.outerConeCos = 0.70f;
                        localLight.color = { 1.0f, 0.90f, 0.70f };
                    }
                    localLight.position = { loc[0], loc[1] + 1.5f, loc[2] };
                    localLight.direction = { 0.0f, -1.0f, 0.0f };
                    m_LocalLights.push_back(localLight);
                }
            }

            // Process SkyAtmosphere component or node
            const SkyAtmosphereComponent* skyComp = ComponentRegistry::Get().GetComponent<SkyAtmosphereComponent>(node.id);
            if ((node.type == SceneNodeType::SkyAtmosphere || skyComp != nullptr) && !skyAtmosphereFound) {
                if (node.visible && (!skyComp || skyComp->enabled)) {
                    skyAtmosphereFound = true;
                    activeSkyProxy.active = true;
                    if (skyComp) {
                        activeSkyProxy.skyIntensity = skyComp->skyIntensity;
                        activeSkyProxy.zenithColor[0] = skyComp->zenithColor[0];
                        activeSkyProxy.zenithColor[1] = skyComp->zenithColor[1];
                        activeSkyProxy.zenithColor[2] = skyComp->zenithColor[2];
                        activeSkyProxy.horizonColor[0] = skyComp->horizonColor[0];
                        activeSkyProxy.horizonColor[1] = skyComp->horizonColor[1];
                        activeSkyProxy.horizonColor[2] = skyComp->horizonColor[2];
                    }
                }
            }

            // Process VolumetricFog component or node
            const VolumetricFogComponent* fogComp = ComponentRegistry::Get().GetComponent<VolumetricFogComponent>(node.id);
            if ((node.type == SceneNodeType::VolumetricFog || fogComp != nullptr) && !volumetricFogFound) {
                if (node.visible && (!fogComp || fogComp->enabled)) {
                    volumetricFogFound = true;
                    activeFogProxy.active = true;
                    if (fogComp) {
                        activeFogProxy.density = fogComp->density;
                        activeFogProxy.heightFalloff = fogComp->heightFalloff;
                        activeFogProxy.color[0] = fogComp->color[0];
                        activeFogProxy.color[1] = fogComp->color[1];
                        activeFogProxy.color[2] = fogComp->color[2];
                        activeFogProxy.startDistance = fogComp->startDistance;
                        activeFogProxy.endDistance = fogComp->endDistance;
                    }
                }
            }

            if (!node.children.empty()) {
                self(self, node.children);
            }
        }
    };

    traverseNodes(traverseNodes, rootNodes);

    zegfx::Color fogColor = activeFogProxy.active ? zegfx::Color(
        static_cast<uint8_t>(activeFogProxy.color[0] * 255.0f),
        static_cast<uint8_t>(activeFogProxy.color[1] * 255.0f),
        static_cast<uint8_t>(activeFogProxy.color[2] * 255.0f), 255
    ) : zegfx::Color(0, 0, 0, 255);

    float fogDensity = activeFogProxy.active ? activeFogProxy.density : 0.0f;
    float fogStart   = activeFogProxy.active ? activeFogProxy.startDistance : 0.0f;
    float fogEnd     = activeFogProxy.active ? activeFogProxy.endDistance : 500.0f;
    bool fogEnabled  = activeFogProxy.active;

    m_HasSkyAtmosphere = skyAtmosphereFound;
    if (!m_HasSkyAtmosphere) {
        // Sky Component Removed/Absent: Pure Black Void
        m_SkyAmbientColor = zegfx::Color(0, 0, 0, 255);
        if (m_Renderer) {
            m_Renderer->setOutdoorLighting(
                zegfx::Color(0, 0, 0, 255),
                zegfx::Color(0, 0, 0, 255),
                0.0f, // 0.0f Intensity -> Sky pass bypassed, Pure Black Void
                fogColor, fogDensity, fogStart, fogEnd, fogEnabled
            );
        }
    } else {
        if (m_Renderer) {
            m_Renderer->setOutdoorLighting(
                zegfx::Color(static_cast<uint8_t>(activeSkyProxy.horizonColor[0] * 255.0f), static_cast<uint8_t>(activeSkyProxy.horizonColor[1] * 255.0f), static_cast<uint8_t>(activeSkyProxy.horizonColor[2] * 255.0f), 255),
                zegfx::Color(static_cast<uint8_t>(activeSkyProxy.zenithColor[0] * 255.0f), static_cast<uint8_t>(activeSkyProxy.zenithColor[1] * 255.0f), static_cast<uint8_t>(activeSkyProxy.zenithColor[2] * 255.0f), 255),
                activeSkyProxy.skyIntensity,
                fogColor, fogDensity, fogStart, fogEnd, fogEnabled
            );
        }
    }

    if (!m_DirectionalLights.empty()) {
        const auto& activeSun = m_DirectionalLights[0];
        if (m_Renderer) {
            m_Renderer->setDirectionalLight(activeSun);
        }
    } else {
        // Strict Component-Based Sunlight: No hardcoded sunlight when directional light is deleted/absent!
        zegfx::DirectionalLightData disabledSun = {};
        disabledSun.direction = { 0.0f, -1.0f, 0.0f };
        disabledSun.illuminanceLux = 0.0f;
        disabledSun.color = { 0.0f, 0.0f, 0.0f };
        if (m_Renderer) {
            m_Renderer->setDirectionalLight(disabledSun);
        }
    }

    // 3. Assemble RenderSceneSnapshot (Directional Sun Light & Scene Mesh Instances like Zenbo)
    zegfx::RenderSceneSnapshot snapshot = {};
    snapshot.opaque = zegfx::Span<const zegfx::RenderInstance>(m_OpaqueInstances.data(), m_OpaqueInstances.size());
    snapshot.directionalLights = zegfx::Span<const zegfx::DirectionalLightData>(m_DirectionalLights.data(), m_DirectionalLights.size());
    snapshot.localLights = zegfx::Span<const zegfx::LocalLightData>(m_LocalLights.data(), m_LocalLights.size());
    snapshot.environment.ambientColor = m_SkyAmbientColor;
    snapshot.environment.intensityMultiplier = m_HasSkyAtmosphere ? activeSkyProxy.skyIntensity : 0.0f;

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
    camera.viewProjection = camera.view * camera.projection;
    EngineEditor::Vec3f camPos = edCam.GetPosition();
    camera.position = { camPos.x, camPos.y, camPos.z };
    camera.nearPlane = 0.1f;
    camera.farPlane = 1000.0f;

    // Render snapshot through bulk mode API
    auto tStartRender = std::chrono::high_resolution_clock::now();
    m_Renderer->render(snapshot, camera, m_Renderer->getSettings(), externalCmdList);
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
    // 1-Second Smooth FPS & Frame-Time Accumulator (Eliminates text flickering)
    static float s_FpsAccumTime = 0.0f;
    static int s_FpsFrameCount = 0;
    static float s_LastReportedFps = 60.0f;
    static float s_LastReportedMs = 16.6f;

    s_FpsAccumTime += deltaTime;
    s_FpsFrameCount++;

    if (s_FpsAccumTime >= 1.0f) {
        s_LastReportedFps = (float)s_FpsFrameCount / s_FpsAccumTime;
        s_LastReportedMs = (s_FpsAccumTime / (float)s_FpsFrameCount) * 1000.0f;
        s_FpsAccumTime = 0.0f;
        s_FpsFrameCount = 0;
    }

    auto& stats = EditorState::Get().stats;
    stats.fps = s_LastReportedFps;
    stats.frameTimeMs = s_LastReportedMs;

    // Query Real System Performance Metrics (RAM, VRAM, GPU Model)
    QuerySystemPerformanceStats(stats.ramUsedGB, stats.vramUsedGB, stats.vramTotalGB, stats.gpuName);

    // Calculate Real Draw Calls & Entity Counts
    zegfx::IRendererDiagnostics* diag = m_Renderer->getDiagnostics();
    if (diag && diag->getDiagnostics().drawCallCount > 0) {
        stats.drawCalls = diag->getDiagnostics().drawCallCount;
        stats.triangleCount = diag->getDiagnostics().triangleCount;
    } else {
        stats.drawCalls = (uint32_t)m_OpaqueInstances.size() + 2; // Opaque meshes + grid + overlay pass
        stats.triangleCount = (uint32_t)m_OpaqueInstances.size() * 12;
    }

    stats.entityCount = (uint32_t)SceneGraph::Get().GetTotalNodeCount();
    stats.apiTag = "ZeGFX v1.0.0 (DX12)";
}

void ZeGFXAdapter::SetOutputRenderTarget(ID3D12Resource* rtResource, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle) {
    m_ActiveRenderTargetResource = rtResource;
    m_ActiveRtvHandle = rtvHandle;
    if (m_Renderer) {
        m_Renderer->setExternalRenderTargetCPU(rtvHandle.ptr);
    }
}

void ZeGFXAdapter::Clear(zegfx::Color color) {
    if (m_Renderer) {
        m_Renderer->clear(color);
    }
}

zegfx::Color ZeGFXAdapter::GetSkyColor() const {
    return m_Renderer ? m_Renderer->getSkyColor() : zegfx::Color(51, 107, 191, 255);
}

void ZeGFXAdapter::Render(ID3D12GraphicsCommandList* cmdList, uint32_t width, uint32_t height, float deltaTime) {
    if (!m_Initialized || !m_Renderer) return;

    m_TimeAccumulator += deltaTime;
    Resize(width, height);

    if (m_Renderer && m_ActiveRtvHandle.ptr != 0) {
        m_Renderer->setExternalRenderTargetCPU(m_ActiveRtvHandle.ptr);
    }

    zegfx::ExternalCmdListHandle handle{};
    if (cmdList != nullptr) {
        handle.native = cmdList;
    } else {
        m_Renderer->beginFrame();
    }

    SyncEngineState(deltaTime, handle);

    if (cmdList == nullptr) {
        m_Renderer->endFrame();
    }

    if (m_PhysicsWorld && deltaTime > 0.0f) {
        constexpr float fixedStep = 1.0f / 60.0f;
        constexpr float maxAccumulatedTime = 0.25f;
        float frameTime = (deltaTime > maxAccumulatedTime) ? maxAccumulatedTime : deltaTime;

        m_PhysicsAccumulator += frameTime;
        while (m_PhysicsAccumulator >= fixedStep) {
            m_PhysicsWorld->StepSimulation(fixedStep);
            m_PhysicsAccumulator -= fixedStep;
        }
    }
}

} // namespace EngineEditor
