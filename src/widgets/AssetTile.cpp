#include "AssetTile.h"
#include "theme/Colors.h"
#include "third_party/IconsFontAwesome6.h"

namespace EngineEditor::Widgets {

bool RenderAssetTile(const char* id, const char* name, const char* typeName, const ImVec4& typeColor, bool isSelected, float width, float height) {
    bool clicked = false;
    const auto& pal = Theme::GetPalette();

    ImVec4 selectedBg = ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.15f);
    ImVec4 selectedBorder = ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.80f);

    ImGui::PushID(id);
    ImGui::BeginGroup();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, isSelected ? selectedBg : pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_Border, isSelected ? selectedBorder : pal.borderSubtle);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, isSelected ? 1.5f : 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));

    ImGui::BeginChild("AssetCardTile", ImVec2(width, height), true, ImGuiWindowFlags_NoScrollbar);

    // Type badge
    ImGui::TextColored(typeColor, "%s", typeName);

    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    // Subtle separator line
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(p.x, p.y), ImVec2(p.x + width - 16.0f, p.y),
        ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    ImGui::Dummy(ImVec2(0.0f, 4.0f));

    // Asset name
    ImGui::TextUnformatted(name);

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