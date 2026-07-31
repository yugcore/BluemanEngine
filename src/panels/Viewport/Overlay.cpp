#include "Overlay.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

namespace EngineEditor::Panels {

void RenderViewportStatsHUD(ImVec2 cursorPos) {
    const auto& pal = Theme::GetPalette();
    const auto& stats = EditorState::Get().stats;

    ImGui::SetNextWindowPos(ImVec2(cursorPos.x + 12.0f, cursorPos.y + 44.0f));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(pal.bgBase.x, pal.bgBase.y, pal.bgBase.z, 0.70f));
    ImGui::PushStyleColor(ImGuiCol_Border, pal.borderSubtle);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    ImGuiWindowFlags hudFlags = ImGuiWindowFlags_NoDecoration |
                                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    if (ImGui::BeginChild("##ViewportStatsHUD", ImVec2(300.0f, 190.0f), true, hudFlags)) {
        // GPU Name
        ImGui::TextColored(pal.accent, "GPU: %s", stats.gpuName.c_str());
        
        // VRAM bar
        ImGui::TextColored(pal.textSecondary, "VRAM: %.1f / %.1f GB (%.0f%%)",
            stats.vramUsedGB, stats.vramTotalGB, (stats.vramUsedGB / stats.vramTotalGB) * 100.0f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, pal.accent);
        ImGui::ProgressBar(stats.vramUsedGB / stats.vramTotalGB, ImVec2(-1.0f, 4.0f), "");
        ImGui::PopStyleColor();

        ImGui::Spacing();
        
        // Detailed stats
        ImGui::TextColored(pal.textSecondary, "Draw Calls: %u", stats.drawCalls);
        ImGui::TextColored(pal.textSecondary, "Upscaler: %s", stats.upscalerMode.c_str());
        ImGui::TextColored(pal.textSecondary, "Ray Tracing: %s", stats.rtxGIStatus.c_str());
        ImGui::TextColored(pal.textSecondary, "%s", stats.volumetricLighting.c_str());
        ImGui::TextColored(pal.textSecondary, "%s", stats.naniteStatus.c_str());
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

} // namespace EngineEditor::Panels
