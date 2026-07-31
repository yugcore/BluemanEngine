#include "PropertyRow.h"
#include "core/EditorState.h"
#include "core/CommandStack.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include <cstdio>

namespace EngineEditor::Widgets {

static TransformData s_EditStartTransform;
static bool s_IsEditingTransform = false;

// Axis color constants matching UE5 convention
static const ImVec4 kAxisColorX = ImVec4(0.80f, 0.18f, 0.18f, 1.00f);
static const ImVec4 kAxisColorY = ImVec4(0.18f, 0.65f, 0.18f, 1.00f);
static const ImVec4 kAxisColorZ = ImVec4(0.18f, 0.38f, 0.80f, 1.00f);
static constexpr float kLabelColumnWidth = Theme::Metrics::labelColumnWidth;
static constexpr float kAxisBtnWidth = 18.0f;
static constexpr float kAxisBtnHeight = 20.0f;

// --- Helper: Render a single axis (button + drag float) with undo support ---
static bool RenderAxisField(const char* axisLabel, const ImVec4& axisColor, float* value,
                            float resetValue, float itemWidth) {
    bool changed = false;
    ImGui::PushItemWidth(itemWidth);
    
    ImGui::PushStyleColor(ImGuiCol_Button, axisColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(axisColor.x + 0.1f, axisColor.y + 0.1f, axisColor.z + 0.1f, 1.0f));
    if (ImGui::Button(axisLabel, ImVec2(kAxisBtnWidth, kAxisBtnHeight))) {
        *value = resetValue;
        changed = true;
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine();
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
    
    // Label with dropdown arrow indicator
    const auto& pal = Theme::GetPalette();
    ImGui::TextColored(pal.textSecondary, "%s", label);
    ImGui::SameLine();
    ImGui::TextColored(pal.textDisabled, "\xE2\x96\xBC");
    
    ImGui::NextColumn();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::intraGroupGap, 0.0f));
    float lockWidth = lockAspect ? 50.0f : 0.0f;
    float itemWidth = (ImGui::GetContentRegionAvail().x - lockWidth - 16.0f) / 3.0f;

    float prevY = values[1];
    float prevZ = values[2];

    // X
    if (RenderAxisField("X", kAxisColorX, &values[0], resetValue, itemWidth)) valueChanged = true;
    ImGui::SameLine();
    // Y
    if (RenderAxisField("Y", kAxisColorY, &values[1], resetValue, itemWidth)) valueChanged = true;
    ImGui::SameLine();
    // Z
    if (RenderAxisField("Z", kAxisColorZ, &values[2], resetValue, itemWidth)) valueChanged = true;

    // Lock aspect ratio button
    if (lockAspect) {
        ImGui::SameLine();
        if (*lockAspect) ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        const char* lockLabel = *lockAspect ? "Locked" : "Lock";
        if (ImGui::Button(lockLabel, ImVec2(lockWidth, kAxisBtnHeight))) *lockAspect = !(*lockAspect);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip(*lockAspect ? "Lock Aspect Ratio (Active)" : "Unlock Aspect Ratio");
        if (*lockAspect) {
            ImGui::PopStyleColor();
            if (values[1] != prevY) values[0] = values[2] = values[1];
            else if (values[2] != prevZ) values[0] = values[1] = values[2];
        }
    }

    ImGui::PopStyleVar();
    ImGui::Columns(1);
    ImGui::PopID();

    return valueChanged;
}

} // namespace EngineEditor::Widgets
