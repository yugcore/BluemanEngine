#include "ViewportPanel.h"
#include "Overlay.h"
#include "Gizmos.h"
#include "render/ViewportRenderer.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio>

namespace EngineEditor {

static int s_PerspectiveIdx = 0;
static int s_QualityIdx = 0;
static int s_ShowFlags = 7;

static void RenderStatusPill(const char* label, const ImVec4& color) {
    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 3.0f));

    ImGui::Button(label);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void RenderViewportPanel(bool* pOpen) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (!ImGui::Begin("Viewport", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImVec2 viewportAvail = ImGui::GetContentRegionAvail();
    uint32_t width = (uint32_t)viewportAvail.x;
    uint32_t height = (uint32_t)viewportAvail.y;

    if (width > 0 && height > 0) {
        ViewportRenderer::Get().Resize(width, height);
    }

    float deltaTime = ImGui::GetIO().DeltaTime;
    ViewportRenderer::Get().RenderScene(deltaTime);

    uint64_t textureID = ViewportRenderer::Get().GetTextureID();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (textureID != 0) {
        ImGui::Image((ImTextureID)textureID, viewportAvail, ImVec2(0, 1), ImVec2(1, 0));
    }

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const auto& pal = Theme::GetPalette();

    // === In-Viewport Control Strip (docked FLUSH to top edge, no floating gap/seam) ===
    float toolbarH = Theme::Metrics::headerHeight;
    {
        float toolbarY = cursorPos.y;
        float toolbarX = cursorPos.x;
        float toolbarW = viewportAvail.x;
        
        drawList->AddRectFilled(
            ImVec2(toolbarX, toolbarY),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            ImGui::ColorConvertFloat4ToU32(pal.bgHeader));
        drawList->AddLine(
            ImVec2(toolbarX, toolbarY + toolbarH),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    }
    
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + Theme::Metrics::panelLeftMargin, cursorPos.y + (toolbarH - Theme::Metrics::rowHeight) * 0.5f));
    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgPanel);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

    // --- Group 1: Perspective / 1:2 / Show ---
    const char* persNames[] = { "Perspective", "Top", "Front", "Side" };
    if (ImGui::Button(persNames[s_PerspectiveIdx])) ImGui::OpenPopup("ViewportPerspPopup");
    if (ImGui::BeginPopup("ViewportPerspPopup")) {
        if (ImGui::MenuItem("Perspective", nullptr, s_PerspectiveIdx == 0)) s_PerspectiveIdx = 0;
        if (ImGui::MenuItem("Top", nullptr, s_PerspectiveIdx == 1)) s_PerspectiveIdx = 1;
        if (ImGui::MenuItem("Front", nullptr, s_PerspectiveIdx == 2)) s_PerspectiveIdx = 2;
        if (ImGui::MenuItem("Side", nullptr, s_PerspectiveIdx == 3)) s_PerspectiveIdx = 3;
        ImGui::EndPopup();
    }

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    const char* qualNames[] = { "1:2", "1:1", "4K" };
    if (ImGui::Button(qualNames[s_QualityIdx])) ImGui::OpenPopup("ViewportQualPopup");
    if (ImGui::BeginPopup("ViewportQualPopup")) {
        if (ImGui::MenuItem("1:2 (Half Res)", nullptr, s_QualityIdx == 0)) s_QualityIdx = 0;
        if (ImGui::MenuItem("1:1 (Full Res)", nullptr, s_QualityIdx == 1)) s_QualityIdx = 1;
        if (ImGui::MenuItem("4K (Ultra HD)", nullptr, s_QualityIdx == 2)) s_QualityIdx = 2;
        ImGui::EndPopup();
    }

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (ImGui::Button("Show")) ImGui::OpenPopup("ViewportShowPopup");
    if (ImGui::BeginPopup("ViewportShowPopup")) {
        bool showGrid = (s_ShowFlags & 1) != 0;
        bool showLights = (s_ShowFlags & 2) != 0;
        bool showGeo = (s_ShowFlags & 4) != 0;
        if (ImGui::Checkbox("Grid", &showGrid)) s_ShowFlags ^= 1;
        if (ImGui::Checkbox("Light Icons", &showLights)) s_ShowFlags ^= 2;
        if (ImGui::Checkbox("Geometry Outlines", &showGeo)) s_ShowFlags ^= 4;
        ImGui::EndPopup();
    }

    // Inter-group separator (groupGap = 18px)
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);

    // --- Group 2: Transform Tools (Move / Rotate / Scale) ---
    auto& gizmo = EditorState::Get().gizmoOp;
    bool mvActive = (gizmo == GizmoOperation::Translate);
    bool rtActive = (gizmo == GizmoOperation::Rotate);
    bool scActive = (gizmo == GizmoOperation::Scale);

    if (mvActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    }
    if (ImGui::Button("Move")) gizmo = GizmoOperation::Translate;
    if (mvActive) ImGui::PopStyleColor(2);

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (rtActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    }
    if (ImGui::Button("Rotate")) gizmo = GizmoOperation::Rotate;
    if (rtActive) ImGui::PopStyleColor(2);

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (scActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    }
    if (ImGui::Button("Scale")) gizmo = GizmoOperation::Scale;
    if (scActive) ImGui::PopStyleColor(2);

    // --- Right-Aligned Status Pills ---
    const auto& stats = EditorState::Get().stats;
    
    char fpsBuf[32]; snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.1f", stats.fps);
    char msBuf[32]; snprintf(msBuf, sizeof(msBuf), "Frame: %.2f ms", stats.frameTimeMs);
    char wpBuf[48]; snprintf(wpBuf, sizeof(wpBuf), "WP: 2 Cells");
    char entBuf[32]; snprintf(entBuf, sizeof(entBuf), "Entities: %u", stats.entityCount);

    const float pillPaddingX = 16.0f;
    const float gap = Theme::Metrics::intraGroupGap;
    float actualPillsWidth = 0.0f;
    actualPillsWidth += ImGui::CalcTextSize(stats.apiTag.c_str()).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(fpsBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(msBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize("DRR: 1:1").x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(wpBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(entBuf).x + pillPaddingX;

    float rightMargin = 20.0f;
    float pillsStart = viewportAvail.x - actualPillsWidth - rightMargin;
    float currentX = ImGui::GetCursorPosX() - cursorPos.x;

    if (pillsStart > currentX + 16.0f) {
        ImGui::SameLine(pillsStart);
    } else {
        ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    }

    RenderStatusPill(stats.apiTag.c_str(), pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(fpsBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(msBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill("DRR: 1:1", pal.accent);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(wpBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    if (ImGui::Button(entBuf)) ImGui::OpenPopup("EntitiesPopup");
    if (ImGui::BeginPopup("EntitiesPopup")) {
        ImGui::Text("Active Scene Entities: %u", stats.entityCount);
        ImGui::Separator();
        ImGui::MenuItem("Select All Entities");
        ImGui::MenuItem("Hide Unselected Entities");
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    // Render Stats Overlay
    Panels::RenderViewportStatsHUD(ImVec2(cursorPos.x, cursorPos.y + toolbarH));

    // Render ImGuizmo
    Panels::RenderViewportGizmos(drawList, cursorPos, viewportAvail);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace EngineEditor
