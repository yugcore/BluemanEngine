#include "CodeBottomDockPanel.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

void RenderCodeBottomDockPanel(bool* pOpen) {
    if (!ImGui::Begin("Terminal", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (ImGui::BeginTabBar("CodeBottomDockTabBar")) {
        if (ImGui::BeginTabItem("Terminal")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Terminal Session Ready [ZeGFX Shell]");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Build Output")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No build active.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Git")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Git status: Clean working tree.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Problems")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "0 Errors | 0 Warnings");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Debug Console")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Debugger disconnected.");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
