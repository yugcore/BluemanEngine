#ifndef THEME_FONTS_H
#define THEME_FONTS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct FontAtlas {
    ImFont* bodyFont          = nullptr; // Tier 3: 12px Regular (Default)
    ImFont* panelTitleFont    = nullptr; // Tier 1: 14px Semibold
    ImFont* sectionHeaderFont = nullptr; // Tier 2: 12px Semibold
    ImFont* secondaryFont     = nullptr; // Tier 4: 10px Regular
    ImFont* monoFont          = nullptr; // Monospace for Console / Output Log
};

FontAtlas SetupFonts();
const FontAtlas& GetFontAtlas();

} // namespace EngineEditor::Theme

#endif // THEME_FONTS_H
