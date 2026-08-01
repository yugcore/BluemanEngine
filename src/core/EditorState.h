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

    // Panel Visibility Defaults
    bool showRenderControlStrip = false;
    bool showContentBrowser = true;
    bool showOutputLog = false;
};

struct MeshStudioData {
    std::string meshName = "";
    std::string meshPath = "";
    uint32_t vertexCount = 0;
    uint32_t triangleCount = 0;
    uint32_t submeshCount = 0;
    float boundsMin[3] = { 0.0f, 0.0f, 0.0f };
    float boundsMax[3] = { 0.0f, 0.0f, 0.0f };
    bool isLoaded = false;
};

struct ShaderStudioData {
    std::string shaderName = "";
    std::string shaderSource = "";
    bool isCompiled = false;
    std::string compileErrorStr = "";
};

struct TextureViewerData {
    std::string textureName = "";
    uint32_t width = 0;
    uint32_t height = 0;
    std::string formatStr = "";
    float sizeMB = 0.0f;
    bool isLoaded = false;
};

struct RenderStats {
    std::string gpuName = "GPU Standby / Unbound";
    float vramUsedGB = 0.0f;
    float vramTotalGB = 0.0f;
    uint32_t triangleCount = 0;
    uint32_t drawCalls = 0;
    uint32_t entityCount = 0;
    float fps = 0.0f;
    float frameTimeMs = 0.0f;
    float ramUsedGB = 0.0f;
    float ramTotalGB = 0.0f;
    float cpuUsagePct = 0.0f;
    std::string upscalerMode = "Off";
    std::string rtxGIStatus = "Standby";
    std::string volumetricLighting = "Disabled";
    std::string naniteStatus = "Inactive";
    std::string apiTag = "DX12";
    std::string csrPolicy = "CSR Policy: Idle";
};

struct EditorState {
    // Workspace Architecture
    WorkspaceMode activeWorkspace = WorkspaceMode::Editor;

    // Status Bar & System Metadata
    EngineStatus status = EngineStatus::Ready;
    std::string gitBranch = "main";
    std::string currentLevelName = "Untitled Scene";
    std::string editorMode = "LevelDesign";
    std::string editorModeName = "Editor Mode: LevelDesign";
    float cpuUsagePct = 0.0f;

    // Render Settings & Stats
    RenderSettings settings;
    RenderStats stats;

    // Studio Data Ports (exposing runtime hooks)
    MeshStudioData meshStudioData;
    ShaderStudioData shaderStudioData;
    TextureViewerData textureViewerData;

    // Active Selection Transform & Details
    TransformData activeTransform;
    SkyAtmosphereProperties skyAtmosphereProps;
    GizmoOperation gizmoOp = GizmoOperation::Translate;

    // Asset Selection State
    std::string selectedFolderPath = "";
    std::string selectedAssetName = "";
    std::string selectedAssetPath = "";
    std::string selectedAssetType = "";

    // Scene Graph Node Selection State
    std::string selectedNodeName = "";
    std::string selectedNodeType = "";

    // Active Code Document State
    std::string activeCodeFileName = "";
    int activeCodeLine = 1;
    int activeCodeColumn = 1;
    int activeCodeErrorsCount = 0;
    bool showNewFileDialog = false;

    // Modal & Panel Visibility Toggles
    bool showProjectWizardModal = false;
    bool showProjectSettingsModal = false;

    // Always Visible (Default)
    bool showViewportPanel = true;
    bool showOutlinerPanel = true;
    bool showDetailsPanel = true;

    // Hidden by Default
    bool showObjectPalettePanel = false;
    bool showMeshStudioPanel = false;
    bool showShaderStudioPanel = false;
    bool showTextureViewerPanel = false;
    bool showMaterialEditorPanel = false;
    bool showAnimationWorkspacePanel = false;
    bool showBlueprintEditorPanel = false;
    bool showAudioEditorPanel = false;
    bool showProfilerPanel = false;
    bool showRenderDocPanel = false;
    bool showGpuDebuggerPanel = false;
    bool showSequencerPanel = false;
    bool showAssetRegistryPanel = false;
    bool showMemoryProfilerPanel = false;
    bool showPackageManagerPanel = false;
    bool showPluginManagerPanel = false;
    bool showLocalizationPanel = false;
    bool showConsoleVariablesPanel = false;
    bool showNavigationBuilderPanel = false;
    bool showLightBakingPanel = false;

    void SetSelection(const std::string& name, const std::string& type, const std::string& path = "") {
        selectedNodeName = name;
        selectedNodeType = type;
        if (!name.empty()) {
            selectedAssetName = name;
            selectedAssetType = type;
            selectedAssetPath = path.empty() ? ("Scene/" + name) : path;
            showDetailsPanel = true; // Automatically opens or expands when an object is selected
        }
    }

    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }
};

} // namespace EngineEditor

#endif // EDITOR_STATE_H
