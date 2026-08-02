#include "MeshStudioPanel.h"
#include "core/EditorState.h"
#include "engine/core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "widgets/PropertyRow.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

static int s_MeshRenderMode = 0; // 0: Lit Solid, 1: Wireframe, 2: Normals, 3: UV Layout
static bool s_ShowCollisionOverlay = true;
static bool s_ShowBoundingBox = true;

void RenderMeshStudioPanel(bool* pOpen) {
    if (!ImGui::Begin("Mesh Studio", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    const auto& meshData = EditorState::Get().meshStudioData;

    if (!meshData.isLoaded && meshData.meshName.empty()) {
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 30.0f);
        ImGui::TextColored(pal.textDisabled, "No Mesh Selected for Inspection.");
        ImGui::Spacing();
        ImGui::TextColored(pal.textSecondary, "Select a mesh asset in the Content Browser to inspect geometry statistics and material slots.");
        ImGui::PopStyleVar();
        ImGui::End();
        return;
    }

    // Active Mesh Name Header Card
    ImGui::TextColored(pal.textSecondary, "ACTIVE MESH ASSET");
    ImGui::TextColored(pal.textPrimary, "%s", meshData.meshName.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Viewport Mode Buttons
    ImGui::TextColored(pal.textSecondary, "Display Mode:");
    const char* modes[] = { "Lit Solid", "Wireframe", "Normals", "UVs" };
    for (int m = 0; m < 4; ++m) {
        if (m > 0) ImGui::SameLine(0.0f, 4.0f);
        bool isSel = (s_MeshRenderMode == m);
        ImGui::PushStyleColor(ImGuiCol_Button, isSel ? pal.accent : pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_Text, isSel ? pal.bgBase : pal.textPrimary);
        if (ImGui::Button(modes[m])) s_MeshRenderMode = m;
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Checkbox("Show Convex Collision Mesh", &s_ShowCollisionOverlay);
    ImGui::SameLine(0.0f, 12.0f);
    ImGui::Checkbox("Show Bounding Box", &s_ShowBoundingBox);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Section 1: Geometry Statistics
    if (ImGui::CollapsingHeader("Geometry Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);
        ImGui::TextColored(pal.textSecondary, "Vertices:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "%u", meshData.vertexCount);

        ImGui::TextColored(pal.textSecondary, "Triangles:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "%u", meshData.triangleCount);

        ImGui::TextColored(pal.textSecondary, "Submeshes:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "%u", meshData.submeshCount);

        ImGui::TextColored(pal.textSecondary, "Bounding Box (m):");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "X: %.1f  Y: %.1f  Z: %.1f", 
            meshData.boundsMax[0] - meshData.boundsMin[0],
            meshData.boundsMax[1] - meshData.boundsMin[1],
            meshData.boundsMax[2] - meshData.boundsMin[2]);
        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Section 2: Material Slots
    if (ImGui::CollapsingHeader("Material Slots", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);
        
        ImGui::TextColored(pal.textSecondary, "Slot 0 [Main Surface]:");
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        if (ImGui::Button("M_PBR_MetallicStructure.mat##Slot0", ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 0.0f))) {
            Logger::Get().Info("[MeshStudio] Material Slot 0 selected.");
        }
        ImGui::PopStyleColor();

        ImGui::TextColored(pal.textSecondary, "Slot 1 [Glass Panels]:");
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        if (ImGui::Button("M_Glass_Refractive.mat##Slot1", ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 0.0f))) {
            Logger::Get().Info("[MeshStudio] Material Slot 1 selected.");
        }
        ImGui::PopStyleColor();

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Section 3: Mesh Processing Actions
    if (ImGui::CollapsingHeader("Geometry Actions", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        if (ImGui::Button("Generate Convex Collision", ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 0.0f))) {
            Logger::Get().Info("[MeshStudio] Convex Collision generated.");
        }
        if (ImGui::Button("Generate LOD Chain (3 Tiers)", ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 0.0f))) {
            Logger::Get().Info("[MeshStudio] LOD Chain generated.");
        }
        if (ImGui::Button("Export Optimized Binary Mesh", ImVec2(ImGui::GetContentRegionAvail().x - 16.0f, 0.0f))) {
            Logger::Get().Info("[MeshStudio] Mesh exported.");
        }
        ImGui::PopStyleColor();
        ImGui::Unindent(8.0f);
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace EngineEditor
