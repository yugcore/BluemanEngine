#include "Fonts.h"
#include <fstream>

namespace EngineEditor::Theme {

static FontAtlas s_FontAtlas;

static bool CheckFileExists(const char* path) {
    std::ifstream f(path);
    return f.good();
}

FontAtlas SetupFonts() {
    ImGuiIO& io = ImGui::GetIO();

    const char* fontSans = "C:\\Windows\\Fonts\\segoeui.ttf";
    const char* fontSansBold = "C:\\Windows\\Fonts\\segoeuib.ttf";
    const char* fontMono = "C:\\Windows\\Fonts\\consola.ttf";

    if (!CheckFileExists(fontSans)) fontSans = "C:\\Windows\\Fonts\\arial.ttf";
    if (!CheckFileExists(fontSansBold)) fontSansBold = fontSans;
    if (!CheckFileExists(fontMono)) fontMono = "C:\\Windows\\Fonts\\lucon.ttf";

    static const ImWchar glyphRanges[] = {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x2BFF, // Arrows, Math, Geometric Shapes, Misc Symbols, Dingbats
        0x25A0, 0x25FF, // Geometric Shapes (for ▶ ⏹ etc)
        0,
    };

    if (CheckFileExists(fontSans)) {
        // Tier 3: Body / Field Labels (14px Regular - Default)
        s_FontAtlas.bodyFont = io.Fonts->AddFontFromFileTTF(fontSans, 14.0f, nullptr, glyphRanges);
        io.FontDefault = s_FontAtlas.bodyFont;

        // Tier 1: Panel / Window Titles (15px Semibold/Bold)
        s_FontAtlas.panelTitleFont = CheckFileExists(fontSansBold) ? 
            io.Fonts->AddFontFromFileTTF(fontSansBold, 15.0f, nullptr, glyphRanges) : io.Fonts->AddFontFromFileTTF(fontSans, 15.0f, nullptr, glyphRanges);

        // Tier 2: Section Headers (15px Semibold/Bold)
        s_FontAtlas.sectionHeaderFont = CheckFileExists(fontSansBold) ? 
            io.Fonts->AddFontFromFileTTF(fontSansBold, 15.0f, nullptr, glyphRanges) : io.Fonts->AddFontFromFileTTF(fontSans, 15.0f, nullptr, glyphRanges);

        // Tier 4: Secondary / Meta / Status Bar (12px Regular)
        s_FontAtlas.secondaryFont = io.Fonts->AddFontFromFileTTF(fontSans, 12.0f, nullptr, glyphRanges);
    } else {
        s_FontAtlas.bodyFont = io.Fonts->AddFontDefault();
        s_FontAtlas.panelTitleFont = s_FontAtlas.bodyFont;
        s_FontAtlas.sectionHeaderFont = s_FontAtlas.bodyFont;
        s_FontAtlas.secondaryFont = s_FontAtlas.bodyFont;
    }

    if (CheckFileExists(fontMono)) {
        s_FontAtlas.monoFont = io.Fonts->AddFontFromFileTTF(fontMono, 13.0f, nullptr, glyphRanges);
    } else {
        s_FontAtlas.monoFont = s_FontAtlas.bodyFont;
    }

    return s_FontAtlas;
}

const FontAtlas& GetFontAtlas() {
    return s_FontAtlas;
}

} // namespace EngineEditor::Theme
