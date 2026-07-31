#include "SearchBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

namespace EngineEditor::Widgets {

bool RenderSearchBar(const char* label, char* buffer, size_t bufferSize, const char* hint, float width) {
    const auto& pal = Theme::GetPalette();
    
    // Frame padding grid alignment
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, pal.bgElevated);
    
    if (width > 0.0f) {
        ImGui::SetNextItemWidth(width);
    } else if (width < 0.0f) {
        ImGui::SetNextItemWidth(width);
    }
    
    bool changed = ImGui::InputTextWithHint(label, hint, buffer, bufferSize);
    
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(1);
    
    return changed;
}

} // namespace EngineEditor::Widgets
