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
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 0.15f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x, color.y, color.z, 0.30f));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));

    ImGui::Button(label);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void RenderViewportPanel(bool* pOpen) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (!ImGui::Begin("Viewport", pOpen)) {
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

    // === In-Viewport Toolbar (semi-transparent overlay) ===
    {
        float toolbarY = cursorPos.y + 6.0f;
        float toolbarX = cursorPos.x + 6.0f;
        float toolbarH = 32.0f;
        float toolbarW = viewportAvail.x - 12.0f;
        
        drawList->AddRectFilled(
            ImVec2(toolbarX, toolbarY),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            ImGui::ColorConvertFloat4ToU32(ImVec4(pal.bgBase.x, pal.bgBase.y, pal.bgBase.z, 0.85f)), 2.0f);
        drawList->AddRect(
            ImVec2(toolbarX, toolbarY),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 2.0f);
    }
    
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + 12.0f, cursorPos.y + 9.0f));
    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

    const char* persNames[] = { "Perspective \xE2\x96\xBC", "Top \xE2\x96\xBC", "Front \xE2\x96\xBC", "Side \xE2\x96\xBC" };
    if (ImGui::Button(persNames[s_PerspectiveIdx])) ImGui::OpenPopup("ViewportPerspPopup");
    if (ImGui::BeginPopup("ViewportPerspPopup")) {
        if (ImGui::MenuItem("Perspective", nullptr, s_PerspectiveIdx == 0)) s_PerspectiveIdx = 0;
        if (ImGui::MenuItem("Top", nullptr, s_PerspectiveIdx == 1)) s_PerspectiveIdx = 1;
        if (ImGui::MenuItem("Front", nullptr, s_PerspectiveIdx == 2)) s_PerspectiveIdx = 2;
        if (ImGui::MenuItem("Side", nullptr, s_PerspectiveIdx == 3)) s_PerspectiveIdx = 3;
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    const char* qualNames[] = { "1:2 \xE2\x96\xBC", "1:1 \xE2\x96\xBC", "4K \xE2\x96\xBC" };
    if (ImGui::Button(qualNames[s_QualityIdx])) ImGui::OpenPopup("ViewportQualPopup");
    if (ImGui::BeginPopup("ViewportQualPopup")) {
        if (ImGui::MenuItem("1:2 (Half Res)", nullptr, s_QualityIdx == 0)) s_QualityIdx = 0;
        if (ImGui::MenuItem("1:1 (Full Res)", nullptr, s_QualityIdx == 1)) s_QualityIdx = 1;
        if (ImGui::MenuItem("4K (Ultra HD)", nullptr, s_QualityIdx == 2)) s_QualityIdx = 2;
        ImGui::EndPopup();
    }

    ImGui::SameLine();
    if (ImGui::Button("Show \xE2\x96\xBC")) ImGui::OpenPopup("ViewportShowPopup");
    if (ImGui::BeginPopup("ViewportShowPopup")) {
        bool showGrid = (s_ShowFlags & 1) != 0;
        bool showLights = (s_ShowFlags & 2) != 0;
        bool showGeo = (s_ShowFlags & 4) != 0;
        if (ImGui::Checkbox("Grid", &showGrid)) s_ShowFlags ^= 1;
        if (ImGui::Checkbox("Light Icons", &showLights)) s_ShowFlags ^= 2;
        if (ImGui::Checkbox("Geometry Outlines", &showGeo)) s_ShowFlags ^= 4;
        ImGui::EndPopup();
    }

    ImGui::SameLine(0.0f, Theme::Metrics::sectionIndent);
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0.0f, Theme::Metrics::sectionIndent);

    // Transform tool buttons in viewport
    auto& gizmo = EditorState::Get().gizmoOp;
    bool mvActive = (gizmo == GizmoOperation::Translate);
    bool rtActive = (gizmo == GizmoOperation::Rotate);
    bool scActive = (gizmo == GizmoOperation::Scale);

    if (mvActive) ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    if (ImGui::Button("Move")) gizmo = GizmoOperation::Translate;
    if (mvActive) ImGui::PopStyleColor();

    ImGui::SameLine();
    if (rtActive) ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    if (ImGui::Button("Rotate")) gizmo = GizmoOperation::Rotate;
    if (rtActive) ImGui::PopStyleColor();

    ImGui::SameLine();
    if (scActive) ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    if (ImGui::Button("Scale")) gizmo = GizmoOperation::Scale;
    if (scActive) ImGui::PopStyleColor();

    // Right-Aligned Status Pills
    const auto& stats = EditorState::Get().stats;
    
    float pillsWidth = 500.0f;
    float pillsStart = viewportAvail.x - pillsWidth - 10.0f;
    if (pillsStart > ImGui::GetCursorPosX() - cursorPos.x + 14.0f) {
        ImGui::SameLine(pillsStart);
    } else {
        ImGui::SameLine();
    }

    RenderStatusPill(stats.apiTag.c_str(), pal.textSecondary);
    ImGui::SameLine();

    char fpsBuf[32]; snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.1f", stats.fps);
    RenderStatusPill(fpsBuf, pal.textSecondary);
    ImGui::SameLine();

    char msBuf[32]; snprintf(msBuf, sizeof(msBuf), "Frame: %.2f ms", stats.frameTimeMs);
    RenderStatusPill(msBuf, pal.textSecondary);
    ImGui::SameLine();

    RenderStatusPill("DRR: 1:1", pal.accent);
    ImGui::SameLine();

    char wpBuf[48]; snprintf(wpBuf, sizeof(wpBuf), "World Partition: 2 Cells");
    RenderStatusPill(wpBuf, pal.textSecondary);
    ImGui::SameLine();

    char entBuf[32]; snprintf(entBuf, sizeof(entBuf), "Entities: %u \xE2\x96\xBC", stats.entityCount);
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
    Panels::RenderViewportStatsHUD(cursorPos);

    // Render ImGuizmo
    Panels::RenderViewportGizmos(drawList, cursorPos, viewportAvail);

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace EngineEditor
