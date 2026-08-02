#include "ImportProgressModal.h"
#include "engine/assets/BackgroundAssetCooker.h"
#include "core/EditorState.h"
#include "theme/Colors.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

void RenderImportProgressModal() {
    auto& cooker = BackgroundAssetCooker::Get();
    bool isCooking = cooker.IsCooking();

    if (!isCooking) return;

    ImGui::OpenPopup("Asset Import Progress##Modal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480.0f, 200.0f), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse;

    if (ImGui::BeginPopupModal("Asset Import Progress##Modal", nullptr, flags)) {
        std::string taskName = cooker.GetCurrentTaskName();
        std::string statusText = cooker.GetCurrentStatusText();
        float progress = cooker.GetCurrentProgress();

        const auto& pal = Theme::GetPalette();

        ImGui::TextColored(pal.accent, "Importing & Cooking Asset Payload");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("File: ");
        ImGui::SameLine();
        ImGui::TextColored(pal.textPrimary, "%s", taskName.c_str());

        ImGui::Spacing();
        char barBuf[64];
        snprintf(barBuf, sizeof(barBuf), "%.0f%%", progress * 100.0f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, pal.accent);
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 20.0f), barBuf);
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::TextDisabled("Stage: %s", statusText.c_str());

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Cancel Import", ImVec2(120.0f, 28.0f))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

} // namespace EngineEditor
