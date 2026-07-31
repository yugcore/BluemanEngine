#include "RunBottomDockPanel.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

void RenderRunBottomDockPanel(bool* pOpen) {
    if (!ImGui::Begin("Logs", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (ImGui::BeginTabBar("RunBottomDockTabBar")) {
        if (ImGui::BeginTabItem("Logs")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Game runtime logs active.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Breakpoints")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "No breakpoints set.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Variables")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Local variables: 0 items.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Profiler")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "Micro-profiler recording.");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("RenderDoc")) {
            ImGui::Spacing();
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
            ImGui::TextColored(pal.textDisabled, "RenderDoc capture integration ready.");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
