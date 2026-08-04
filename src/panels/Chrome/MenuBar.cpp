#include "MenuBar.h"
#include "render/ZeGFXAdapter.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"
#include "panels/Chrome/CustomTitleBar.h"
#include "panels/Chrome/Toolbar.h"
#include "panels/Chrome/StatusBar.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "core/CommandStack.h"
#include "core/WindowsFileDialog.h"
#include "engine/core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include <imgui.h>
#include <filesystem>

namespace EngineEditor {

// Shared menu content rendering — used by both standalone and inline modes
void RenderMenuBarContents() {
    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, pal.textDisabled);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));   // idle top-level item: no box
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, pal.bgHeader);   // hovered item / open submenu
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, pal.accent);   // clicked/held
    ImGui::PushStyleColor(ImGuiCol_Separator, pal.borderSubtle);
    ImGui::PushStyleColor(ImGuiCol_Border, pal.borderSubtle);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::groupGap, Theme::Metrics::intraGroupGap));   // gaps between top-level menus
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::sectionIndent, 6.0f)); // dropdown inner padding
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Project Wizard...", "Ctrl+Shift+N")) EditorState::Get().showProjectWizardModal = true;
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
            SceneGraph::Get().Clear();
            EditorState::Get().SetSelection("", "");
            EditorState::Get().currentLevelName = "Untitled Scene";
            EditorState::Get().currentScenePath = "";
            Logger::Get().Info("[Menu] File > New Scene created.");
        }
        if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
            auto files = WindowsFileDialog::OpenFileDialog(FileDialogType::OpenScene, "Open Blueman Scene Map", false);
            if (!files.empty()) {
                if (SceneGraph::Get().LoadFromFile(files[0])) {
                    EditorState::Get().currentScenePath = files[0];
                    std::filesystem::path p(files[0]);
                    EditorState::Get().currentLevelName = p.filename().string();
                    EditorState::Get().AddRecentScene(files[0]);
                    EditorState::Get().SetSelection("", "");
                    Logger::Get().Info("[Menu] Loaded scene from " + files[0]);
                }
            }
        }
        if (ImGui::BeginMenu("Recent Scenes")) {
            const auto& recents = EditorState::Get().recentScenes;
            if (recents.empty()) {
                ImGui::TextDisabled("No recent scenes");
            } else {
                for (const auto& scenePath : recents) {
                    std::string label = std::filesystem::path(scenePath).filename().string();
                    if (ImGui::MenuItem(label.c_str())) {
                        if (SceneGraph::Get().LoadFromFile(scenePath)) {
                            EditorState::Get().currentScenePath = scenePath;
                            EditorState::Get().currentLevelName = label;
                            EditorState::Get().AddRecentScene(scenePath);
                            Logger::Get().Info("[Menu] Loaded recent scene: " + scenePath);
                        }
                    }
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            if (EditorState::Get().currentScenePath.empty()) {
                auto files = WindowsFileDialog::OpenFileDialog(FileDialogType::SaveScene, "Save Blueman Scene Map", false);
                if (!files.empty()) {
                    EditorState::Get().currentScenePath = files[0];
                    std::filesystem::path p(files[0]);
                    EditorState::Get().currentLevelName = p.filename().string();
                    EditorState::Get().AddRecentScene(files[0]);
                }
            }
            if (!EditorState::Get().currentScenePath.empty()) {
                if (SceneGraph::Get().SaveToFile(EditorState::Get().currentScenePath)) {
                    EditorState::Get().AddRecentScene(EditorState::Get().currentScenePath);
                    Logger::Get().Info("[Menu] Saved scene to " + EditorState::Get().currentScenePath);
                }
            }
        }
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
            auto files = WindowsFileDialog::OpenFileDialog(FileDialogType::SaveScene, "Save Blueman Scene Map As", false);
            if (!files.empty()) {
                EditorState::Get().currentScenePath = files[0];
                std::filesystem::path p(files[0]);
                EditorState::Get().currentLevelName = p.filename().string();
                EditorState::Get().AddRecentScene(files[0]);
                SceneGraph::Get().SaveToFile(files[0]);
                Logger::Get().Info("[Menu] Saved scene as " + files[0]);
            }
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Import Assets...", "Ctrl+I")) {
            EditorState::Get().TriggerImportFileDialog();
            Logger::Get().Info("[Menu] File > Import Assets dialog opened.");
        }
        if (ImGui::MenuItem("Import Heightmap (PNG/RAW)...")) {
            EditorState::Get().TriggerImportHeightmapDialog();
            Logger::Get().Info("[Menu] File > Import Heightmap requested.");
        }
        if (ImGui::MenuItem("Import Asset with Options...")) {
            EditorState::Get().TriggerImportFileDialog();
            Logger::Get().Info("[Menu] File > Import Asset with Options requested.");
        }
        if (ImGui::BeginMenu("Export Selection")) {
            bool hasSelection = !EditorState::Get().selectedNodeName.empty();
            if (ImGui::MenuItem("Export Selection as FBX...", nullptr, false, hasSelection)) {
                Logger::Get().Info("[Menu] Exporting selection as FBX...");
            }
            if (ImGui::MenuItem("Export Selection as glTF...", nullptr, false, hasSelection)) {
                Logger::Get().Info("[Menu] Exporting selection as glTF...");
            }
            ImGui::EndMenu();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Autosave Settings...")) {
            EditorState::Get().showAutosaveSettingsModal = true;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) {
            Logger::Get().Info("[Menu] File > Exit requested.");
            exit(0);
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) CommandStack::Get().Undo();
        if (ImGui::MenuItem("Redo", "Ctrl+Y")) CommandStack::Get().Redo();
        ImGui::Separator();
        bool hasSelection = !EditorState::Get().selectedNodeName.empty();
        if (ImGui::MenuItem("Cut", "Ctrl+X", false, hasSelection)) {
            const SceneNode* target = SceneGraph::Get().FindNode(EditorState::Get().selectedNodeName);
            if (target) {
                SceneGraph::Get().SetClipboard(*target);
                SceneGraph::Get().RemoveNode(EditorState::Get().selectedNodeName);
                EditorState::Get().ClearSelection();
                Logger::Get().Info("[Menu] Cut node to clipboard.");
            }
        }
        if (ImGui::MenuItem("Copy", "Ctrl+C", false, hasSelection)) {
            const SceneNode* target = SceneGraph::Get().FindNode(EditorState::Get().selectedNodeName);
            if (target) {
                SceneGraph::Get().SetClipboard(*target);
                Logger::Get().Info("[Menu] Copied node to clipboard.");
            }
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V", false, SceneGraph::Get().HasClipboard())) {
            SceneNode* pasted = SceneGraph::Get().PasteClipboard();
            if (pasted) {
                EditorState::Get().SetSelection(pasted->name, SceneGraph::GetTypeName(pasted->type));
                Logger::Get().Info("[Menu] Pasted node from clipboard: " + pasted->name);
            }
        }
        if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, hasSelection)) {
            SceneNode* dup = SceneGraph::Get().DuplicateNode(EditorState::Get().selectedNodeName);
            if (dup) {
                EditorState::Get().SetSelection(dup->name, SceneGraph::GetTypeName(dup->type));
                Logger::Get().Info("[Menu] Duplicated node: " + dup->name);
            }
        }
        if (ImGui::MenuItem("Delete", "Del", false, hasSelection)) {
            SceneGraph::Get().RemoveNode(EditorState::Get().selectedNodeName);
            EditorState::Get().ClearSelection();
            Logger::Get().Info("[Menu] Deleted selected node.");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Project Settings...")) EditorState::Get().showProjectSettingsModal = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Create")) {
        if (ImGui::MenuItem("Empty GameObject")) {
            SceneNode emptyNode;
            emptyNode.name = "EmptyActor_" + std::to_string(rand() % 1000);
            emptyNode.type = SceneNodeType::Actor;
            SceneGraph::Get().AddNode(emptyNode);
            EditorState::Get().SetSelection(emptyNode.name, "Actor");
            Logger::Get().Info("[Menu] Create > Empty GameObject: " + emptyNode.name);
        }
        if (ImGui::BeginMenu("3D Object")) {
            if (ImGui::MenuItem("Cube")) {
                SceneNode cubeNode;
                cubeNode.name = "Cube_" + std::to_string(rand() % 1000);
                cubeNode.type = SceneNodeType::Actor;
                cubeNode.location[0] = 0.0f; cubeNode.location[1] = 1.0f; cubeNode.location[2] = 0.0f;
                cubeNode.meshPath = "Engine/DefaultCube";
                cubeNode.materialPath = "DefaultPBRMaterial";
                SceneGraph::Get().AddNode(cubeNode);
                EditorState::Get().SetSelection(cubeNode.name, "Actor");
            }
            if (ImGui::MenuItem("Sphere")) {
                SceneNode sphereNode;
                sphereNode.name = "Sphere_" + std::to_string(rand() % 1000);
                sphereNode.type = SceneNodeType::Actor;
                sphereNode.location[0] = 0.0f; sphereNode.location[1] = 1.0f; sphereNode.location[2] = 0.0f;
                sphereNode.meshPath = "Engine/DefaultSphere";
                sphereNode.materialPath = "DefaultPBRMaterial";
                SceneGraph::Get().AddNode(sphereNode);
                EditorState::Get().SetSelection(sphereNode.name, "Actor");
            }
            if (ImGui::MenuItem("Cylinder")) {
                SceneNode cylNode;
                cylNode.name = "Cylinder_" + std::to_string(rand() % 1000);
                cylNode.type = SceneNodeType::Actor;
                cylNode.location[0] = 0.0f; cylNode.location[1] = 1.0f; cylNode.location[2] = 0.0f;
                cylNode.meshPath = "Engine/DefaultCylinder";
                cylNode.materialPath = "DefaultPBRMaterial";
                SceneGraph::Get().AddNode(cylNode);
                EditorState::Get().SetSelection(cylNode.name, "Actor");
            }
            if (ImGui::MenuItem("Plane")) {
                SceneNode planeNode;
                planeNode.name = "Plane_" + std::to_string(rand() % 1000);
                planeNode.type = SceneNodeType::Actor;
                planeNode.location[0] = 0.0f; planeNode.location[1] = 0.05f; planeNode.location[2] = 0.0f;
                planeNode.meshPath = "Engine/DefaultPlane";
                planeNode.materialPath = "DefaultPBRMaterial";
                SceneGraph::Get().AddNode(planeNode);
                EditorState::Get().SetSelection(planeNode.name, "Actor");
            }
            if (ImGui::MenuItem("Cone")) {
                SceneNode coneNode;
                coneNode.name = "Cone_" + std::to_string(rand() % 1000);
                coneNode.type = SceneNodeType::Actor;
                coneNode.location[0] = 0.0f; coneNode.location[1] = 1.0f; coneNode.location[2] = 0.0f;
                coneNode.meshPath = "Engine/DefaultCone";
                coneNode.materialPath = "DefaultPBRMaterial";
                SceneGraph::Get().AddNode(coneNode);
                EditorState::Get().SetSelection(coneNode.name, "Actor");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Light")) {
            if (ImGui::MenuItem("Directional Light")) {
                SceneNode sunNode;
                sunNode.name = "DirectionalSunLight_" + std::to_string(rand() % 1000);
                sunNode.type = SceneNodeType::Light;
                sunNode.location[0] = 0.0f; sunNode.location[1] = 10.0f; sunNode.location[2] = 0.0f;
                sunNode.rotation[0] = 53.0f; sunNode.rotation[1] = -59.0f; sunNode.rotation[2] = 0.0f;
                SceneGraph::Get().AddNode(sunNode);
                EditorState::Get().SetSelection(sunNode.name, "Light");
            }
            if (ImGui::MenuItem("Point Light")) {
                SceneNode ptNode;
                ptNode.name = "PointLight_" + std::to_string(rand() % 1000);
                ptNode.type = SceneNodeType::Light;
                ptNode.location[0] = 0.0f; ptNode.location[1] = 3.0f; ptNode.location[2] = 0.0f;
                SceneGraph::Get().AddNode(ptNode);
                EditorState::Get().SetSelection(ptNode.name, "Light");
            }
            if (ImGui::MenuItem("Spot Light")) {
                SceneNode spotNode;
                spotNode.name = "SpotLight_" + std::to_string(rand() % 1000);
                spotNode.type = SceneNodeType::Light;
                spotNode.location[0] = 0.0f; spotNode.location[1] = 3.0f; spotNode.location[2] = 0.0f;
                spotNode.rotation[0] = 45.0f; spotNode.rotation[1] = 0.0f; spotNode.rotation[2] = 0.0f;
                SceneGraph::Get().AddNode(spotNode);
                EditorState::Get().SetSelection(spotNode.name, "Light");
            }
            if (ImGui::MenuItem("SkyAtmosphere")) {
                SceneNode skyNode;
                skyNode.name = "SkyAtmosphere_" + std::to_string(rand() % 1000);
                skyNode.type = SceneNodeType::SkyAtmosphere;
                SceneGraph::Get().AddNode(skyNode);
                EditorState::Get().SetSelection(skyNode.name, "SkyAtmosphere");
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Audio")) {
            if (ImGui::MenuItem("Audio Source")) {
                SceneNode audioNode;
                audioNode.name = "AudioSource_" + std::to_string(rand() % 1000);
                audioNode.type = SceneNodeType::Audio;
                SceneGraph::Get().AddNode(audioNode);
                EditorState::Get().SetSelection(audioNode.name, "Audio");
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
        if (ImGui::BeginMenu("Lighting Debug / Render Pass")) {
            static int s_RenderPassMode = 0;
            const char* renderPassNames[] = { "Lit (PBR)", "Unlit", "Depth Buffer", "World Normals", "Roughness & Metallic", "Volumetric Fog Grid", "Cascaded Shadow Atlas" };
            int debugMap[] = { 0, 1, 7, 3, 4, 10, 8 };
            for (int r = 0; r < 7; ++r) {
                if (ImGui::MenuItem(renderPassNames[r], nullptr, s_RenderPassMode == r)) {
                    s_RenderPassMode = r;
                    EngineEditor::ZeGFXAdapter::Get().SetLightingDebugMode(debugMap[r]);
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window")) {
        auto& state = EditorState::Get();

        ImGui::TextDisabled("Always Visible (Default)");
        ImGui::MenuItem("Viewport", nullptr, &state.showViewportPanel);
        ImGui::MenuItem("Outliner", nullptr, &state.showOutlinerPanel);
        ImGui::MenuItem("Content Browser", nullptr, &state.settings.showContentBrowser);
        ImGui::MenuItem("Details / Inspector", nullptr, &state.showDetailsPanel);

        ImGui::Separator();
        ImGui::TextDisabled("Hidden by Default Panels");
        ImGui::MenuItem("Object Palette", nullptr, &state.showObjectPalettePanel);
        ImGui::MenuItem("Output Log", nullptr, &state.settings.showOutputLog);
        ImGui::MenuItem("Profiler", nullptr, &state.showProfilerPanel);
        ImGui::MenuItem("RenderDoc Integrator", nullptr, &state.showRenderDocPanel);
        ImGui::MenuItem("GPU Debugger", nullptr, &state.showGpuDebuggerPanel);
        ImGui::MenuItem("Memory Profiler", nullptr, &state.showMemoryProfilerPanel);
        ImGui::MenuItem("Package Manager", nullptr, &state.showPackageManagerPanel);
        ImGui::MenuItem("Plugin Manager", nullptr, &state.showPluginManagerPanel);
        ImGui::MenuItem("Localization", nullptr, &state.showLocalizationPanel);
        ImGui::MenuItem("Console Variables", nullptr, &state.showConsoleVariablesPanel);
        ImGui::MenuItem("Render Control Strip", nullptr, &state.settings.showRenderControlStrip);

        ImGui::Separator();
        ImGui::TextDisabled("Context-Sensitive & Editors");
        ImGui::MenuItem("Material Editor", nullptr, &state.showMaterialEditorPanel);
        ImGui::MenuItem("Blueprint / Visual Script", nullptr, &state.showBlueprintEditorPanel);
        ImGui::MenuItem("Shader Studio", nullptr, &state.showShaderStudioPanel);
        ImGui::MenuItem("Texture Viewer", nullptr, &state.showTextureViewerPanel);
        ImGui::MenuItem("Mesh Studio", nullptr, &state.showMeshStudioPanel);
        ImGui::MenuItem("Animation Workspace", nullptr, &state.showAnimationWorkspacePanel);
        ImGui::MenuItem("Audio Mixer", nullptr, &state.showAudioEditorPanel);
        ImGui::MenuItem("Sequencer / Timeline", nullptr, &state.showSequencerPanel);

        ImGui::Separator();
        if (ImGui::MenuItem("Reset Layout")) {
            Layout::RequestLayoutReset();
            Logger::Get().Info("[Menu] Window > Reset Layout executed.");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
        auto& state = EditorState::Get();
        if (ImGui::MenuItem("Material Editor")) state.showMaterialEditorPanel = true;
        if (ImGui::MenuItem("Blueprint Editor")) state.showBlueprintEditorPanel = true;
        if (ImGui::MenuItem("Animation Workspace")) state.showAnimationWorkspacePanel = true;
        if (ImGui::MenuItem("Audio Mixer")) state.showAudioEditorPanel = true;
        if (ImGui::MenuItem("Shader Studio")) state.showShaderStudioPanel = true;
        if (ImGui::MenuItem("Texture Inspector")) state.showTextureViewerPanel = true;
        ImGui::Separator();
        if (ImGui::MenuItem("Asset Registry", nullptr, &state.showAssetRegistryPanel)) Logger::Get().Info("[Menu] Tools > Asset Registry");
        if (ImGui::MenuItem("Navigation Builder", nullptr, &state.showNavigationBuilderPanel)) Logger::Get().Info("[Menu] Tools > Navigation Builder");
        if (ImGui::MenuItem("Light Baking", nullptr, &state.showLightBakingPanel)) Logger::Get().Info("[Menu] Tools > Light Baking");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Build")) {
        if (ImGui::MenuItem("Build Geometry")) {
            EditorState::Get().status = EngineStatus::Building;
            Logger::Get().Info("[Menu] Build > Geometry started.");
            EditorState::Get().status = EngineStatus::Ready;
        }
        if (ImGui::MenuItem("Build Lighting (DXR)")) {
            EditorState::Get().status = EngineStatus::Building;
            Logger::Get().Info("[Menu] Build > Lighting DXR pass started.");
            EditorState::Get().status = EngineStatus::Ready;
        }
        if (ImGui::MenuItem("Build Navigation Grid")) {
            EditorState::Get().status = EngineStatus::Building;
            Logger::Get().Info("[Menu] Build > Navigation AI grid started.");
            EditorState::Get().status = EngineStatus::Ready;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Build All", "Ctrl+Shift+B")) {
            EditorState::Get().status = EngineStatus::Building;
            Logger::Get().Info("[Menu] Build All executed (Geometry + DXR Lighting + NavGrid).");
            EditorState::Get().status = EngineStatus::Ready;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Documentation")) Logger::Get().Info("[Menu] Help > Opening Blueman Documentation.");
        if (ImGui::MenuItem("About Blueman Engine")) EditorState::Get().showAboutModal = true;
        ImGui::EndMenu();
    }

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(8);
}

// Standalone menu bar (no longer called when using unified title bar)
void RenderMenuBar() {
    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, pal.bgBase);
    if (ImGui::BeginMainMenuBar()) {
        RenderMenuBarContents();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor();
}

} // namespace EngineEditor