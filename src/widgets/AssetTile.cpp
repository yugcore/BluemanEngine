#include "AssetTile.h"
#include "theme/Colors.h"
#include "third_party/IconsFontAwesome6.h"
#include <imgui_internal.h>
#include <cmath>

namespace EngineEditor::Widgets {

bool RenderAssetTile(const char* id, const char* name, const char* typeName, const ImVec4& typeColor, bool isSelected, float width, float height) {
    bool clicked = false;
    const auto& pal = Theme::GetPalette();

    ImVec4 cardBg = isSelected ? ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.22f) : pal.bgElevated;
    ImVec4 cardBorder = isSelected ? pal.accent : pal.borderSubtle;

    ImGui::PushID(id);
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, cardBg);
    ImGui::PushStyleColor(ImGuiCol_Border, cardBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, isSelected ? 1.5f : 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 6.0f));

    ImGui::BeginChild("AssetCardTile", ImVec2(width, height), true, ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 cardMin = ImGui::GetWindowPos();

    // 1. Square Thumbnail Box (top 86px)
    float thumbH = 86.0f;
    ImVec2 thumbMin = ImVec2(cardMin.x + 6.0f, cardMin.y + 6.0f);
    ImVec2 thumbMax = ImVec2(cardMin.x + width - 6.0f, cardMin.y + 6.0f + thumbH);

    // Thumbnail inner background (sharp 0.0f corners)
    dl->AddRectFilled(thumbMin, thumbMax, IM_COL32(22, 24, 28, 255), 0.0f);
    dl->AddRect(thumbMin, thumbMax, IM_COL32(45, 48, 56, 255), 0.0f);

    // Render Material Sphere Graphic or Mesh Silhouette inside thumbnail
    ImVec2 thumbCenter = ImVec2((thumbMin.x + thumbMax.x) * 0.5f, (thumbMin.y + thumbMax.y) * 0.5f);
    float radius = 26.0f;

    // Draw Material Sphere Shading with vibrant highlight
    dl->AddCircleFilled(thumbCenter, radius, IM_COL32(190, 195, 205, 255));
    dl->AddCircleFilled(ImVec2(thumbCenter.x - 7.0f, thumbCenter.y - 7.0f), radius * 0.45f, IM_COL32(250, 250, 255, 240));
    dl->AddCircle(thumbCenter, radius, IM_COL32(110, 115, 125, 255), 0, 1.5f);

    // 2. Colored Type Stripe Line below thumbnail
    float stripeY = thumbMax.y + 4.0f;
    dl->AddLine(
        ImVec2(thumbMin.x, stripeY),
        ImVec2(thumbMax.x, stripeY),
        ImGui::ColorConvertFloat4ToU32(typeColor), 3.0f);

    // 3. Asset Name Label with 6px margin below stripe
    ImGui::SetCursorScreenPos(ImVec2(thumbMin.x, stripeY + 6.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? pal.textPrimary : pal.textSecondary);
    
    // Display name cleanly
    ImGui::TextUnformatted(name);
    
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (ImGui::IsItemClicked()) {
        clicked = true;
    }

    ImGui::EndGroup();
    ImGui::PopID();

    return clicked;
}

} // namespace EngineEditor::Widgets