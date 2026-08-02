#ifndef EDITOR_STATE_H
#define EDITOR_STATE_H

#include <string>
#include <cstdint>
#include "EditorCamera.h"
#include "SceneGraph.h"

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

enum class TransformSpace {
    World,
    Local
};

struct SnapSettings {
    bool enableTranslate = true;
    float translateSnap = 1.0f;
    bool enableRotate = true;
    float rotateSnap = 5.0f;
    bool enableScale = true;
    float scaleSnap = 0.25f;
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

struct ZeGFXFogSettings {
    bool enableVolumetric = true;
    float density = 0.02f;
    float anisotropy = 0.5f; // G-factor
    float maxDistance = 128.0f;
    float color[3] = { 0.50f, 0.60f, 0.70f };
};

struct ZeGFXShadowSettings {
    int cascadeResolution = 2048;
    int cascadeCount = 4;
    float maxDistance = 150.0f;
    float constantBias = 0.0002f;
    float slopeBias = 0.0008f;
    float normalBias = 0.40f;
    float filterSoftness = 1.00f;
};

struct ZeGFXAOSettings {
    int mode = 1; // 0: SSAO, 1: GTAO, 2: DXR Raytraced AO
    float radius = 1.50f;
    float intensity = 1.00f;
    bool temporalFiltering = true;
};

struct ZeGFXPostFXSettings {
    int tonemapOperator = 0; // 0: ACES, 1: Reinhard, 2: Uncharted2, 3: Linear
    float exposureEV = 0.00f;
    bool autoExposure = true;
    float contrast = 1.00f;
    float saturation = 1.00f;
    float temperature = 6500.0f;
    float tint = 0.00f;
    float bloomIntensity = 0.50f;
    float bloomThreshold = 1.00f;
};

struct ZeGFXMaterialData {
    std::string materialName = "Default_PBR_Material";
    float baseColor[4] = { 0.80f, 0.80f, 0.80f, 1.00f };
    float roughness = 0.40f;
    float metallic = 0.00f;
    float specular = 0.50f;
    float emissiveIntensity = 0.00f;
    float emissiveColor[3] = { 1.00f, 1.00f, 1.00f };

    std::string albedoMap = "textures/default_albedo.ztex";
    std::string normalMap = "textures/default_normal.ztex";
    std::string roughnessMap = "textures/default_roughness.ztex";
    std::string metallicMap = "textures/default_metallic.ztex";
    std::string emissiveMap = "textures/default_emissive.ztex";
    std::string occlusionMap = "textures/default_ao.ztex";
};

struct ZeGFXTerrainSettings {
    int gridWidth = 512;
    int gridHeight = 512;
    float cellSize = 2.0f; // 512 * 2.0 = 1024m x 1024m (1KM Playable Area)
    float heightScale = 45.0f;
    bool centerPivot = true;
    int maxLodLevel = 4;
};

struct ZeGFXFoliageSettings {
    bool enableGPUCulling = true;
    bool enableExecuteIndirect = true;
    int maxFoliageInstances = 250000;
    float cullDistanceMeters = 800.0f;
    float treeDensity = 1.0f;
    float grassDensity = 2.5f;
};

struct RenderSettings {
    // Quality Presets (0: Low, 1: Medium, 2: High)
    int qualityPreset = 1;

    // Presentation & VSync Controls
    bool enableVSync = false;

    // Hardware Ray Tracing (DXR) & Probe GI (Disabled by default for 100+ FPS)
    bool rtGI = false;
    int giRaysPerProbe = 64;
    int giProbesUpdatedPerFrame = 32;
    bool rtAO = false;
    bool rtReflections = false;
    float dsrScale = 1.00f;
    bool dsrEnabled = true;

    // ZeGFX Engine Specific Feature Options
    ZeGFXFogSettings fog;
    ZeGFXShadowSettings shadow;
    ZeGFXAOSettings ao;
    ZeGFXPostFXSettings postFX;
    ZeGFXTerrainSettings terrain;
    ZeGFXFoliageSettings foliage;
    ZeGFXMaterialData activeMaterial;

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

struct ShaderStudioDiagnostic {
    std::string severity = "ERROR"; // "ERROR", "WARNING", "NOTE"
    int line = 0;
    int column = 0;
    std::string message = "";
};

struct ShaderStudioData {
    std::string shaderName = "CustomSurfaceShader.hlsl";
    std::string shaderSource = "";
    std::string entryPoint = "PSMain";
    std::string targetProfile = "ps_6_0";
    int targetProfileIdx = 0; // 0: ps_6_0, 1: vs_6_0, 2: cs_6_0

    bool isCompiled = false;
    bool lastCompileSucceeded = false;
    std::string compileStatusMsg = "Ready for live compilation";
    uint64_t compileTimeMs = 0;
    size_t dxilBytecodeSize = 0;
    std::string compilerVersion = "DXC";

    std::vector<ShaderStudioDiagnostic> diagnostics;
    bool autoCompileOnEdit = false;
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
    EditorCamera camera;
    TransformSpace activeTransformSpace = TransformSpace::World;
    SnapSettings snapSettings;
    TransformData activeTransform;
    SkyAtmosphereProperties skyAtmosphereProps;
    GizmoOperation gizmoOp = GizmoOperation::Translate;

    // Asset Selection State
    std::string selectedFolderPath = "";
    std::string selectedAssetName = "";
    std::string selectedAssetPath = "";
    std::string selectedAssetType = "";

    // Scene Graph Node Selection State (Multi-selection enabled)
    std::string selectedNodeName = "";
    std::string selectedNodeType = "";
    std::vector<std::string> selectedNodeNames;

    // Viewport Special Modes & Tool States
    bool isPivotEditingMode = false;
    float customPivotOffset[3] = { 0.0f, 0.0f, 0.0f };
    bool isIsolationMode = false;
    bool isMeasurementToolActive = false;
    Vec3f measureStartPos{ 0.0f, 0.0f, 0.0f };
    Vec3f measureEndPos{ 0.0f, 0.0f, 0.0f };

    // Active Code Document State
    std::string activeCodeFileName = "";
    int activeCodeLine = 1;
    int activeCodeColumn = 1;
    int activeCodeErrorsCount = 0;
    bool showNewFileDialog = false;

    std::string currentScenePath = "";
    std::vector<std::string> recentScenes;
    bool showAutosaveSettingsModal = false;

    void AddRecentScene(const std::string& path) {
        if (path.empty()) return;
        auto it = std::find(recentScenes.begin(), recentScenes.end(), path);
        if (it != recentScenes.end()) recentScenes.erase(it);
        recentScenes.insert(recentScenes.begin(), path);
        if (recentScenes.size() > 10) recentScenes.pop_back();
    }

    // Modal & Panel Visibility Toggles
    bool showProjectWizardModal = false;
    bool showProjectSettingsModal = false;
    bool requestImportFileDialog = false;

    void TriggerImportFileDialog() { requestImportFileDialog = true; }

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

    // Simulation & File State
    bool isSimulating = false;
    bool isPaused = false;
    bool showAboutModal = false;

    bool IsSelected(const std::string& name) const {
        if (name.empty()) return false;
        if (selectedNodeName == name) return true;
        return std::find(selectedNodeNames.begin(), selectedNodeNames.end(), name) != selectedNodeNames.end();
    }

    void ClearSelection() {
        selectedNodeName.clear();
        selectedNodeType.clear();
        selectedNodeNames.clear();
        activeTransform = TransformData();
    }

    void SetSelection(const std::string& name, const std::string& type, const std::string& path = "") {
        selectedNodeNames.clear();
        if (name.empty()) {
            selectedNodeName.clear();
            selectedNodeType.clear();
            return;
        }
        selectedNodeNames.push_back(name);
        selectedNodeName = name;
        selectedNodeType = type;
        
        selectedAssetName = name;
        selectedAssetType = type;
        selectedAssetPath = path.empty() ? ("Scene/" + name) : path;
        showDetailsPanel = true;

        const SceneNode* node = SceneGraph::Get().FindNode(name);
        if (node) {
            activeTransform.location[0] = node->location[0];
            activeTransform.location[1] = node->location[1];
            activeTransform.location[2] = node->location[2];

            activeTransform.rotation[0] = node->rotation[0];
            activeTransform.rotation[1] = node->rotation[1];
            activeTransform.rotation[2] = node->rotation[2];

            activeTransform.scale[0] = node->scale[0];
            activeTransform.scale[1] = node->scale[1];
            activeTransform.scale[2] = node->scale[2];
        }
    }

    void AddSelection(const std::string& name, const std::string& type = "Actor") {
        if (name.empty()) return;
        if (!IsSelected(name)) {
            selectedNodeNames.push_back(name);
        }
        selectedNodeName = name;
        selectedNodeType = type;
        showDetailsPanel = true;
    }

    void ToggleSelection(const std::string& name, const std::string& type = "Actor") {
        if (name.empty()) return;
        auto it = std::find(selectedNodeNames.begin(), selectedNodeNames.end(), name);
        if (it != selectedNodeNames.end()) {
            selectedNodeNames.erase(it);
            if (selectedNodeName == name) {
                selectedNodeName = selectedNodeNames.empty() ? "" : selectedNodeNames.back();
            }
        } else {
            selectedNodeNames.push_back(name);
            selectedNodeName = name;
            selectedNodeType = type;
        }
    }

    static EditorState& Get() {
        static EditorState instance;
        return instance;
    }
};

} // namespace EngineEditor

#endif // EDITOR_STATE_H
