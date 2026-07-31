#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <string>
#include <cstdint>

namespace EngineEditor {

enum class WorkspaceMode {
    Editor,
    Codebase,
    Run
};

enum class GizmoOperation {
    Translate,
    Rotate,
    Scale
};

enum class EngineStatus {
    Ready,
    Building,
    Error
};

struct TransformData {
    float location[3] = { 0.0f, 0.0f, 0.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3]    = { 1.0f, 1.0f, 1.0f };
    bool lockAspect   = false;
};

struct SkyAtmosphereProperties {
    float rayleighScattering = 0.058f;
    float aerosolScattering  = 0.004f;
    float aerosolAbsorption  = 0.001f;
    float atmosphereHeightKm = 60.0f;
    float aerialPerspectiveDistanceScale = 1.00f;
};

struct RenderSettings {
    // Quality Presets (0: Low, 1: Medium, 2: High)
    int qualityPreset = 2;

    // Hardware Ray Tracing (DXR)
    bool rtGI = true;
    bool rtAO = true;
    bool rtReflections = true;
    float dsrScale = 1.00f;
    bool dsrEnabled = true;

    // World Partition & Spatial Grid
    float cellSizeMeters = 150.0f;
    float streamingRadiusMeters = 600.0f;
    int activeStreamedLevel = 4;
    int totalSpatialScale = 2048;

    // Nanite & Mesh Shader Pipeline
    bool naniteClusterCulling = true;
    bool naniteFrustumCulling = true;
    bool meshShaderPipeline = true;
    uint32_t naniteClustersDrawn = 84520;
    uint32_t naniteTrianglesCulled = 1240000;

    // Quick Isolation Tests (MRQ Floating)
    bool isoGeometry = false;
    bool isoTextures = false;
    bool isoLighting = false;
    bool isoShadows = false;
    bool isoDXR = false;
    bool isoMaterials = false;
    bool isoMeshShader = false;
    bool isoPostProcess = false;

    // Panel Visibility
    bool showRenderControlStrip = true;
    bool showContentBrowser = true;
    bool showOutputLog = true;
};

struct RenderStats {
    std::string gpuName = "NVIDIA GeForce RTX 4080 (Lovelace)";
    float vramUsedGB = 10.2f;
    float vramTotalGB = 16.0f;
    uint32_t triangleCount = 3214567;
    uint32_t drawCalls = 1845;
    uint32_t entityCount = 346;
    float fps = 185.2f;
    float frameTimeMs = 5.41f;
    float ramUsedGB = 6.4f;
    float ramTotalGB = 32.0f;
    float cpuUsagePct = 14.2f;
    std::string upscalerMode = "DLSS 3.5 Quality (Frame Gen)";
    std::string rtxGIStatus = "RTX GI Ultra (4 Bounces)";
    std::string volumetricLighting = "Volumetric: Enabled";
    std::string naniteStatus = "Nanite: Active";
    std::string apiTag = "DR12";
    std::string csrPolicy = "CSR Policy: Traversals (MS) 1.1 Active (Ray Tracing + Solve)";
};

struct EditorState {
    // Workspace Architecture
    WorkspaceMode activeWorkspace = WorkspaceMode::Editor;

    // Status Bar & System Metadata (Phase 8)
    EngineStatus status = EngineStatus::Ready;
    std::string gitBranch = "feature/volumetrics";
    std::string currentLevelName = "Default Level";
    std::string editorMode = "LevelDesign";
    std::string editorModeName = "Editor Mode: LevelDesign";
    float cpuUsagePct = 14.2f;

    // Render Settings & Stats (Phases 3 & 4)
    RenderSettings settings;
    RenderStats stats;

    // Active Selection Transform & Details (Phase 6 & 9)
    TransformData activeTransform;
    SkyAtmosphereProperties skyAtmosphereProps;
    GizmoOperation gizmoOp = GizmoOperation::Translate;

    // Asset Selection State (Phase 2)
    std::string selectedFolderPath = "ZeGFX Workspace/Blueman Cooked Assets/Meshes";
    std::string selectedAssetName = "";
    std::string selectedAssetPath = "";
    std::string selectedAssetType = "";

    // Scene Graph Node Selection State (Phase 5 & 9)
    std::string selectedNodeName = "";
    std::string selectedNodeType = "";

    void SetSelection(const std::string& name, const std::string& type, const std::string& path = "") {
        selectedNodeName = name;
        selectedNodeType = type;
        if (!name.empty()) {
            selectedAssetName = name;
            selectedAssetType = type;
            selectedAssetPath = path.empty() ? ("Scene/" + name) : path;
        }
    }

    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }
};

} // namespace EngineEditor

#endif // EDITOR_STATE_H
