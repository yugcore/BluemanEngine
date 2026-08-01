#include "Fonts.h"
#include "IconsFontAwesome6.h"
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

    // FontAwesome 6 glyph range (for icon merge)
    static const ImWchar iconGlyphRanges[] = {
        (ImWchar)ICON_FA_MIN, (ImWchar)ICON_FA_MAX,
        0,
    };

    // Candidate paths for FontAwesome 6 Free Solid .ttf
    const char* faFontPaths[] = {
        "assets/fonts/fa-solid-900.ttf",
        "../assets/fonts/fa-solid-900.ttf",
        "C:\\Windows\\Fonts\\fa-solid-900.ttf",
    };
    const char* faFontPath = nullptr;
    for (const auto& p : faFontPaths) {
        if (CheckFileExists(p)) {
            faFontPath = p;
            break;
        }
    }

    ImFontConfig fontCfg;
    fontCfg.OversampleH = 3;
    fontCfg.OversampleV = 3;
    fontCfg.PixelSnapH = true;

    ImFontConfig iconCfg;
    iconCfg.MergeMode = true;
    iconCfg.OversampleH = 2;
    iconCfg.OversampleV = 2;
    iconCfg.PixelSnapH = true;
    iconCfg.GlyphMinAdvanceX = 16.0f; // Monospace-width icons

    if (CheckFileExists(fontSans)) {
        // Tier 3: Body / Field Labels (18.5px Regular base size)
        s_FontAtlas.bodyFont = io.Fonts->AddFontFromFileTTF(fontSans, 18.5f, &fontCfg, glyphRanges);
        // Merge FontAwesome icons into body font
        if (faFontPath) io.Fonts->AddFontFromFileTTF(faFontPath, 16.0f, &iconCfg, iconGlyphRanges);
        io.FontDefault = s_FontAtlas.bodyFont;

        // Tier 1: Panel / Window Titles (21.0px Bold - 20-22px semi-bold/bold range)
        s_FontAtlas.panelTitleFont = CheckFileExists(fontSansBold) ? 
            io.Fonts->AddFontFromFileTTF(fontSansBold, 21.0f, &fontCfg, glyphRanges) : io.Fonts->AddFontFromFileTTF(fontSans, 21.0f, &fontCfg, glyphRanges);
        if (faFontPath) io.Fonts->AddFontFromFileTTF(faFontPath, 18.0f, &iconCfg, iconGlyphRanges);

        // Tier 2: Section Headers (18.0px Bold)
        s_FontAtlas.sectionHeaderFont = CheckFileExists(fontSansBold) ? 
            io.Fonts->AddFontFromFileTTF(fontSansBold, 18.0f, &fontCfg, glyphRanges) : io.Fonts->AddFontFromFileTTF(fontSans, 18.0f, &fontCfg, glyphRanges);
        if (faFontPath) io.Fonts->AddFontFromFileTTF(faFontPath, 16.0f, &iconCfg, iconGlyphRanges);

        // Tier 4: Secondary / Meta / Status Bar (15.0px Regular)
        s_FontAtlas.secondaryFont = io.Fonts->AddFontFromFileTTF(fontSans, 15.0f, &fontCfg, glyphRanges);
        if (faFontPath) io.Fonts->AddFontFromFileTTF(faFontPath, 13.0f, &iconCfg, iconGlyphRanges);
    } else {
        s_FontAtlas.bodyFont = io.Fonts->AddFontDefault();
        s_FontAtlas.panelTitleFont = s_FontAtlas.bodyFont;
        s_FontAtlas.sectionHeaderFont = s_FontAtlas.bodyFont;
        s_FontAtlas.secondaryFont = s_FontAtlas.bodyFont;
    }

    if (CheckFileExists(fontMono)) {
        // Monospace Font (15.5px for Log Console, FPS/stats, and numeric readouts)
        s_FontAtlas.monoFont = io.Fonts->AddFontFromFileTTF(fontMono, 15.5f, &fontCfg, glyphRanges);
        if (faFontPath) io.Fonts->AddFontFromFileTTF(faFontPath, 14.0f, &iconCfg, iconGlyphRanges);
    } else {
        s_FontAtlas.monoFont = s_FontAtlas.bodyFont;
    }

    return s_FontAtlas;
}

const FontAtlas& GetFontAtlas() {
    return s_FontAtlas;
}

} // namespace EngineEditor::Theme
