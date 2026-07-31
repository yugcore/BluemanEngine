#include "CodeSymbolsPanel.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

void RenderCodeSymbolsPanel(bool* pOpen) {
    if (!ImGui::Begin("Symbols", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (ImGui::BeginTabBar("CodeSymbolsTabBar")) {
        if (ImGui::BeginTabItem("Symbols")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No file open.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("References")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No file open.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Outline")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No file open.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Call Hierarchy")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No file open.");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
