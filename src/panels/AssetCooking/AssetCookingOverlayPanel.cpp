#include "AssetCookingOverlayPanel.h"
#include "engine/assets/BackgroundAssetCooker.h"
#include <imgui.h>

namespace EngineEditor {

void RenderAssetCookingOverlay(BackgroundAssetCooker& cooker) {
    cooker.Update();

    CookingStatus status = cooker.GetCookingStatus();

    ImGuiIO& io = ImGui::GetIO();
    float padding = 16.0f;
    ImVec2 workPos = io.DisplaySize;
    ImVec2 windowPos = ImVec2(workPos.x - padding, workPos.y - padding);

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                            ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoSavedSettings |
                            ImGuiWindowFlags_NoFocusOnAppearing |
                            ImGuiWindowFlags_NoNav |
                            ImGuiWindowFlags_NoMove;

    if (status.isCooking || !status.notifications.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.52f, 0.88f, 0.60f));

        if (ImGui::Begin("##AssetCookingOverlay", nullptr, flags)) {
            if (status.isCooking) {
                ImGui::TextColored(ImVec4(0.28f, 0.68f, 1.00f, 1.00f), "[Asset Cooker]");
                ImGui::SameLine();
                ImGui::TextUnformatted(status.currentTaskName.c_str());

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.18f, 0.58f, 0.94f, 1.00f));
                char barBuf[64];
                snprintf(barBuf, sizeof(barBuf), "%.0f%%", status.currentProgress * 100.0f);
                ImGui::ProgressBar(status.currentProgress, ImVec2(220.0f, 14.0f), barBuf);
                ImGui::PopStyleColor();

                ImGui::TextDisabled("%s", status.currentStatusText.c_str());
            }

            for (const auto& notif : status.notifications) {
                if (status.isCooking) ImGui::Separator();
                if (notif.isSuccess) {
                    ImGui::TextColored(ImVec4(0.22f, 0.85f, 0.44f, 1.00f), "[SUCCESS] %s", notif.text.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.95f, 0.32f, 0.32f, 1.00f), "[ERROR] %s", notif.text.c_str());
                }
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
}

} // namespace EngineEditor
