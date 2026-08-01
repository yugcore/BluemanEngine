#include "MeshStudioPanel.h"
#include "core/EditorState.h"
#include "core/Logger.h"
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

    // Active Mesh Name Header Card
    ImGui::TextColored(pal.textSecondary, "ACTIVE MESH ASSET");
    ImGui::TextColored(pal.textPrimary, "SM_SciFi_Corridor_Chunk01.fbx");
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
        ImGui::TextColored(pal.textPrimary, "14,280");

        ImGui::TextColored(pal.textSecondary, "Triangles:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "28,400");

        ImGui::TextColored(pal.textSecondary, "Submeshes:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "2");

        ImGui::TextColored(pal.textSecondary, "UV Channels:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "2 (Base Color, Lightmap)");

        ImGui::TextColored(pal.textSecondary, "Bounding Box (m):");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "X: 4.2  Y: 2.8  Z: 3.5");

        ImGui::TextColored(pal.textSecondary, "Collision Type:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "Convex Hull (16 Vertices)");
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
