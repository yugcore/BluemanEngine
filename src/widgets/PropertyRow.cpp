#include "PropertyRow.h"
#include "core/EditorState.h"
#include "core/CommandStack.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio>

namespace EngineEditor::Widgets {

static TransformData s_EditStartTransform;
static bool s_IsEditingTransform = false;

// Axis color constants matching UE5 convention
static const ImVec4 kAxisColorX = ImVec4(0.88f, 0.28f, 0.28f, 1.00f);
static const ImVec4 kAxisColorY = ImVec4(0.24f, 0.72f, 0.42f, 1.00f);
static const ImVec4 kAxisColorZ = ImVec4(0.28f, 0.52f, 0.90f, 1.00f);
static constexpr float kLabelColumnWidth = 90.0f;
static constexpr float kAxisChipWidth = 20.0f;
static constexpr float kAxisChipHeight = 22.0f;

// --- Helper: Render a single axis (chip + drag float) with undo support ---
static bool RenderAxisField(const char* axisLabel, const ImVec4& axisColor, float* value,
                            float resetValue, float dragWidth) {
    bool changed = false;
    const auto& pal = Theme::GetPalette();
    ImDrawList* dl = ImGui::GetWindowDrawList();

    ImVec2 chipMin = ImGui::GetCursorScreenPos();
    ImVec2 chipMax = ImVec2(chipMin.x + kAxisChipWidth, chipMin.y + kAxisChipHeight);

    // Interaction target for axis reset chip
    ImGui::ItemSize(ImVec2(kAxisChipWidth, kAxisChipHeight));
    ImGui::ItemAdd(ImRect(chipMin, chipMax), ImGui::GetID(axisLabel));

    if (ImGui::IsItemClicked()) {
        *value = resetValue;
        changed = true;
    }

    // Colored Chip Background & Subtle Border
    dl->AddRectFilled(chipMin, chipMax, ImGui::ColorConvertFloat4ToU32(axisColor), 3.0f);
    dl->AddRect(chipMin, chipMax, ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 3.0f, 0, 1.0f);

    // Dead-centered text label (X / Y / Z)
    ImVec2 txtSz = ImGui::CalcTextSize(axisLabel);
    ImVec2 txtPos = ImVec2(chipMin.x + (kAxisChipWidth - txtSz.x) * 0.5f, chipMin.y + (kAxisChipHeight - txtSz.y) * 0.5f);
    dl->AddText(txtPos, ImGui::ColorConvertFloat4ToU32(pal.bgBase), axisLabel);

    ImGui::SameLine(0.0f, 3.0f);
    ImGui::PushItemWidth(dragWidth);
    char dragId[16];
    snprintf(dragId, sizeof(dragId), "##%s", axisLabel);
    if (ImGui::DragFloat(dragId, value, 0.1f, 0.0f, 0.0f, "%.2f")) changed = true;
    
    // Undo/Redo support
    if (ImGui::IsItemActivated()) {
        s_EditStartTransform = EditorState::Get().activeTransform;
        s_IsEditingTransform = true;
    }
    if (ImGui::IsItemDeactivatedAfterEdit() && s_IsEditingTransform) {
        s_IsEditingTransform = false;
        CommandStack::Get().PushAndExecute(std::make_shared<TransformChangeCommand>(
            EditorState::Get().selectedNodeName, s_EditStartTransform, EditorState::Get().activeTransform
        ));
    }
    
    ImGui::PopItemWidth();
    return changed;
}

bool RenderVector3PropertyRow(const char* label, float values[3], float resetValue, bool* lockAspect) {
    bool valueChanged = false;

    ImGui::PushID(label);

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, kLabelColumnWidth);
    
    // Left label
    const auto& pal = Theme::GetPalette();
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (kAxisChipHeight - ImGui::GetTextLineHeight()) * 0.5f);
    ImGui::TextColored(pal.textSecondary, "%s", label);
    
    ImGui::NextColumn();

    float availWidth = ImGui::GetContentRegionAvail().x;
    float lockWidth = lockAspect ? 22.0f : 0.0f;
    float colGutter = 4.0f;
    float totalAvailForCols = availWidth - (lockAspect ? (lockWidth + colGutter) : 0.0f);

    float colWidth = (totalAvailForCols - 2.0f * colGutter) / 3.0f;
    float dragWidth = colWidth - kAxisChipWidth - 3.0f;
    if (dragWidth < 20.0f) dragWidth = 20.0f;

    float prevY = values[1];
    float prevZ = values[2];

    // X Column
    if (RenderAxisField("X", kAxisColorX, &values[0], resetValue, dragWidth)) valueChanged = true;
    ImGui::SameLine(0.0f, colGutter);
    // Y Column
    if (RenderAxisField("Y", kAxisColorY, &values[1], resetValue, dragWidth)) valueChanged = true;
    ImGui::SameLine(0.0f, colGutter);
    // Z Column
    if (RenderAxisField("Z", kAxisColorZ, &values[2], resetValue, dragWidth)) valueChanged = true;

    // Vector Padlock Aspect Ratio Button (22px fixed width)
    if (lockAspect) {
        ImGui::SameLine(0.0f, colGutter);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        ImVec2 lMin = ImGui::GetCursorScreenPos();
        ImVec2 lMax = ImVec2(lMin.x + lockWidth, lMin.y + kAxisChipHeight);

        ImGui::ItemSize(ImVec2(lockWidth, kAxisChipHeight));
        ImGui::ItemAdd(ImRect(lMin, lMax), ImGui::GetID("##LockAspectBtn"));

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        if (clicked) {
            *lockAspect = !(*lockAspect);
        }

        ImU32 lockBg = ImGui::ColorConvertFloat4ToU32(*lockAspect ? pal.accent : (hovered ? pal.bgElevated : pal.bgHeader));
        ImU32 lockFg = ImGui::ColorConvertFloat4ToU32(*lockAspect ? pal.bgBase : pal.textDisabled);

        dl->AddRectFilled(lMin, lMax, lockBg, 3.0f);
        dl->AddRect(lMin, lMax, ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 3.0f, 0, 1.0f);

        // Vector Lock Icon (Padlock body + arch)
        float cx = lMin.x + lockWidth * 0.5f;
        float cy = lMin.y + kAxisChipHeight * 0.5f;
        dl->AddRectFilled(ImVec2(cx - 3.5f, cy - 1.0f), ImVec2(cx + 3.5f, cy + 4.5f), lockFg, 1.0f);
        dl->AddCircle(ImVec2(cx, cy - 1.5f), 2.5f, lockFg, 12, 1.2f);

        if (hovered) ImGui::SetTooltip(*lockAspect ? "Unlock Aspect Ratio" : "Lock Aspect Ratio");

        if (*lockAspect) {
            if (values[1] != prevY) values[0] = values[2] = values[1];
            else if (values[2] != prevZ) values[0] = values[1] = values[2];
        }
    }

    ImGui::Columns(1);
    ImGui::PopID();

    return valueChanged;
}

} // namespace EngineEditor::Widgets
