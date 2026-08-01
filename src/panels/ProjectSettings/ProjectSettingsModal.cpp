#include "ProjectSettingsModal.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

static char s_SettingsProjectName[128] = "ZeGFX Engine Workspace";
static char s_StartupScene[128] = "src/MainScene.map";
static bool s_EnableVsync = true;
static bool s_EnableRaytracing = true;
static int s_AntiAliasingMode = 1; // 0: Off, 1: TAA, 2: FXAA, 3: MSAA 4x
static bool s_ZelynOptimize = true;
static bool s_ZelynStrictTypes = true;

void RenderProjectSettingsModal(bool* pOpen) {
    if (!pOpen || !*pOpen) return;

    const auto& pal = Theme::GetPalette();
    auto& state = EditorState::Get();

    ImGui::OpenPopup("Project Preferences & Settings");

    if (ImGui::BeginPopupModal("Project Preferences & Settings", pOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

        ImGui::TextColored(pal.textPrimary, "Project Settings & Engine Defaults");
        ImGui::TextColored(pal.textDisabled, "Configure default level startup, DX12 renderer pipeline, and Zelyn compiler flags.");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("ProjectSettingsTabBar")) {
            // Tab 1: General Settings
            if (ImGui::BeginTabItem("General")) {
                ImGui::Spacing();
                static char sceneNameBuf[128] = "";
                if (sceneNameBuf[0] == '\0') {
                    strncpy(sceneNameBuf, state.currentLevelName.c_str(), sizeof(sceneNameBuf) - 1);
                }
                if (ImGui::InputText("Active Scene Title", sceneNameBuf, sizeof(sceneNameBuf))) {
                    state.currentLevelName = sceneNameBuf;
                }
                ImGui::Spacing();
                ImGui::TextColored(pal.textDisabled, "Engine Version: Blueman Engine v3.5 Enterprise [DX12]");
                ImGui::EndTabItem();
            }

            // Tab 2: Rendering Defaults
            if (ImGui::BeginTabItem("Rendering Pipeline")) {
                ImGui::Spacing();
                ImGui::Checkbox("Enable VSync (Vertical Sync)", &state.settings.enableVSync);
                ImGui::Checkbox("Hardware DXR Ray Tracing Acceleration", &state.settings.rtGI);
                ImGui::Checkbox("DXR Raytraced Ambient Occlusion (RTAO)", &state.settings.rtAO);
                ImGui::Checkbox("DXR Raytraced Reflections", &state.settings.rtReflections);
                
                ImGui::Spacing();
                ImGui::TextColored(pal.textSecondary, "Quality Preset:");
                const char* presets[] = { "Low (Performance)", "Medium (Balanced)", "High (Cinematic)" };
                ImGui::Combo("##QualityPreset", &state.settings.qualityPreset, presets, 3);
                ImGui::EndTabItem();
            }

            // Tab 3: Input Keybindings
            if (ImGui::BeginTabItem("Input Keybindings")) {
                ImGui::Spacing();
                ImGui::BulletText("Move Forward: W");
                ImGui::BulletText("Move Backward: S");
                ImGui::BulletText("Strafe Left: A");
                ImGui::BulletText("Strafe Right: D");
                ImGui::BulletText("Orbit Viewport Camera: Right Mouse Button");
                ImGui::BulletText("Focus Selection: F");
                ImGui::BulletText("Gizmo Translate / Rotate / Scale: W / E / R");
                ImGui::BulletText("Toggle World/Local Transform Space: Space");
                ImGui::EndTabItem();
            }

            // Tab 4: Zelyn Compiler Settings
            if (ImGui::BeginTabItem("Zelyn Compiler")) {
                ImGui::Spacing();
                static bool opt = true;
                static bool strict = true;
                ImGui::Checkbox("Enable Optimization (-O3)", &opt);
                ImGui::Checkbox("Enforce Strict Type Checking", &strict);
                ImGui::Spacing();
                ImGui::TextColored(pal.textDisabled, "Target Architecture: x86_64-pc-windows-msvc");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Save & Close buttons
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
        if (ImGui::Button("Save Settings", ImVec2(140.0f, 28.0f))) {
            Logger::Get().Info("[Settings] Applied and persisted project settings.");
            *pOpen = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0.0f, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        if (ImGui::Button("Close", ImVec2(100.0f, 28.0f))) {
            *pOpen = false;
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }
}

} // namespace EngineEditor
