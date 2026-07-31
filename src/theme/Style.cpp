#include "Style.h"
#include "Fonts.h"

namespace EngineEditor::Theme {

void ApplyMasterStyle(const Palette& palette, const Metrics& metrics) {
    ImGuiStyle& style = ImGui::GetStyle();
    ApplyColors(style, palette);
    ApplyMetrics(style, metrics);
    SetupFonts();
}

} // namespace EngineEditor::Theme
