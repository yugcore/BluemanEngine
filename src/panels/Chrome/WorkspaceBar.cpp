#include "WorkspaceBar.h"
#include "CustomTitleBar.h"
#include "StatusBar.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>

namespace EngineEditor {

static constexpr float kWorkspaceBarHeight = 32.0f;

float GetWorkspaceBarTotalHeight() {
    return kWorkspaceBarHeight;
}

void RenderWorkspaceBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const auto& pal = Theme::GetPalette();

    // Position workspace switcher bar directly below custom title bar
    float titleBarBottom = viewport->Pos.y + GetTitleBarTotalHeight();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, titleBarBottom));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kWorkspaceBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::panelLeftMargin, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::intraGroupGap, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgHeader);

    if (ImGui::Begin("##WorkspaceBarWindow", nullptr, flags)) {
        auto& state = EditorState::Get();

        auto RenderTab = [&](const char* label, WorkspaceMode mode) {
            bool isActive = (state.activeWorkspace == mode);
            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.accentHover);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accentActive);
                ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
                if (Theme::GetFontAtlas().sectionHeaderFont) {
                    ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accent);
                ImGui::PushStyleColor(ImGuiCol_Text, pal.textSecondary);
            }

            if (ImGui::Button(label, ImVec2(0.0f, 24.0f))) {
                if (state.activeWorkspace != mode) {
                    state.activeWorkspace = mode;
                    Logger::Get().Info(std::string("[WorkspaceBar] Swapped workspace to ") + label);
                }
            }

            if (isActive) {
                if (Theme::GetFontAtlas().sectionHeaderFont) {
                    ImGui::PopFont();
                }
            }
            ImGui::PopStyleColor(4);
        };

        // Center the Editor / Codebase / Run tabs in the workspace switcher bar
        float totalTabsWidth = 72.0f + 88.0f + 56.0f + (2.0f * Theme::Metrics::intraGroupGap);
        float centerX = (ImGui::GetWindowWidth() - totalTabsWidth) * 0.5f;
        if (centerX > Theme::Metrics::panelLeftMargin) {
            ImGui::SetCursorPosX(centerX);
        }

        RenderTab("Editor", WorkspaceMode::Editor);
        ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
        RenderTab("Codebase", WorkspaceMode::Codebase);
        ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
        RenderTab("Run", WorkspaceMode::Run);

        // Right-aligned Settings stub button
        float settingsWidth = 80.0f;
        float settingsStart = ImGui::GetWindowWidth() - settingsWidth - Theme::Metrics::panelLeftMargin;
        if (settingsStart > ImGui::GetCursorPosX()) {
            ImGui::SameLine(settingsStart);
        } else {
            ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textSecondary);
        if (ImGui::Button("Settings", ImVec2(settingsWidth, 24.0f))) {
            Logger::Get().Info("[WorkspaceBar] Settings clicked.");
        }
        ImGui::PopStyleColor(3);
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor
