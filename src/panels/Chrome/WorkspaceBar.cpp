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
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18.0f, 4.0f));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));

            if (isActive) {
                ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
                if (Theme::GetFontAtlas().sectionHeaderFont) {
                    ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Text, pal.textSecondary);
            }

            ImVec2 bMin = ImGui::GetCursorScreenPos();

            if (ImGui::Button(label)) {
                if (state.activeWorkspace != mode) {
                    state.activeWorkspace = mode;
                    Logger::Get().Info(std::string("[WorkspaceBar] Swapped workspace to ") + label);
                }
            }

            ImVec2 bMax = ImGui::GetItemRectMax();

            if (isActive) {
                // Subtle 2px bottom accent indicator line for active tab
                ImGui::GetWindowDrawList()->AddLine(
                    ImVec2(bMin.x + 4.0f, bMax.y),
                    ImVec2(bMax.x - 4.0f, bMax.y),
                    ImGui::ColorConvertFloat4ToU32(pal.accent), 2.0f);

                if (Theme::GetFontAtlas().sectionHeaderFont) {
                    ImGui::PopFont();
                }
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
        };

        // Center the Editor / Codebase / Run tabs dynamically in the workspace switcher bar
        float eW = ImGui::CalcTextSize("Editor").x + 36.0f;
        float cW = ImGui::CalcTextSize("Codebase").x + 36.0f;
        float rW = ImGui::CalcTextSize("Run").x + 36.0f;
        float totalTabsWidth = eW + cW + rW + (2.0f * Theme::Metrics::intraGroupGap);
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

        ImGui::PushStyleColor(ImGuiCol_Text, pal.textSecondary);
        float textY = ImGui::GetCursorPosY() + (24.0f - ImGui::GetTextLineHeight()) * 0.5f;
        ImGui::SetCursorPosY(textY);
        ImGui::TextColored(pal.textSecondary, "Settings");
        if (ImGui::IsItemHovered()) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            ImGui::SetTooltip("Open Workspace Settings");
        }
        if (ImGui::IsItemClicked()) {
            Logger::Get().Info("[WorkspaceBar] Settings clicked.");
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor
