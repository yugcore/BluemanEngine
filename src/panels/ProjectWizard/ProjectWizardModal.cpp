#include "ProjectWizardModal.h"
#include "engine/core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

static char s_ProjectName[128] = "MyBluemanGame";
static char s_ProjectPath[256] = "C:/Projects/BluemanGames/";
static int s_SelectedTemplateIndex = 0; // 0: Empty Game, 1: DX12 Volumetric Sample, 2: Zelyn Scripting Demo

void RenderProjectWizardModal(bool* pOpen) {
    if (!pOpen || !*pOpen) return;

    const auto& pal = Theme::GetPalette();

    ImGui::OpenPopup("Project Wizard & Class Generator");

    if (ImGui::BeginPopupModal("Project Wizard & Class Generator", pOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

        ImGui::TextColored(pal.textPrimary, "Create New Blueman Engine Project or Class");
        ImGui::TextColored(pal.textDisabled, "Select a project template or scaffold C++ / Zelyn modules.");
        ImGui::Separator();
        ImGui::Spacing();

        // Section 1: Template Selection
        ImGui::TextColored(pal.textSecondary, "Select Project Template:");
        const char* templates[] = { "Empty Game Project", "DX12 Volumetric Lighting Sample", "Zelyn Scripting Demo" };
        
        for (int t = 0; t < 3; ++t) {
            bool isSel = (s_SelectedTemplateIndex == t);
            ImGui::PushStyleColor(ImGuiCol_Button, isSel ? pal.accent : pal.bgHeader);
            ImGui::PushStyleColor(ImGuiCol_Text, isSel ? pal.bgBase : pal.textPrimary);
            if (ImGui::Button(templates[t], ImVec2(240.0f, 32.0f))) {
                s_SelectedTemplateIndex = t;
            }
            ImGui::PopStyleColor(2);
            if (t < 2) ImGui::SameLine(0.0f, 8.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Section 2: Name & Directory Location
        ImGui::InputText("Project Name", s_ProjectName, sizeof(s_ProjectName));
        ImGui::InputText("Target Location", s_ProjectPath, sizeof(s_ProjectPath));

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Action Buttons
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
        if (ImGui::Button("Create Project Workspace", ImVec2(200.0f, 30.0f))) {
            Logger::Get().Info("[Wizard] Created new project workspace: '" + std::string(s_ProjectName) + "'");
            *pOpen = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::SameLine(0.0f, 8.0f);

        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        if (ImGui::Button("Cancel", ImVec2(100.0f, 30.0f))) {
            *pOpen = false;
        }
        ImGui::PopStyleColor();

        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }
}

} // namespace EngineEditor
