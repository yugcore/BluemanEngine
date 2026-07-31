#ifndef THEME_COLORS_H
#define THEME_COLORS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct Palette {
    // ── Background ramp (true grayscale, no tint) ─────────────────────
    ImVec4 bgBase               = ImVec4(0.067f, 0.067f, 0.067f, 1.00f); // #111111
    ImVec4 bgPanel              = ImVec4(0.098f, 0.098f, 0.098f, 1.00f); // #191919
    ImVec4 bgHeader             = ImVec4(0.141f, 0.141f, 0.141f, 1.00f); // #242424
    ImVec4 bgElevated           = ImVec4(0.184f, 0.184f, 0.184f, 1.00f); // #2F2F2F

    // ── Border ────────────────────────────────────────────────────────
    ImVec4 borderSubtle         = ImVec4(0.200f, 0.200f, 0.200f, 0.50f); // #333333 @ 50%

    // ── Single accent (neutral off-white) ─────────────────────────────
    ImVec4 accent               = ImVec4(0.800f, 0.800f, 0.800f, 1.00f); // #CCCCCC
    ImVec4 accentHover          = ImVec4(0.900f, 0.900f, 0.900f, 1.00f); // #E6E6E6
    ImVec4 accentActive         = ImVec4(0.650f, 0.650f, 0.650f, 1.00f); // #A6A6A6

    // ── Text ──────────────────────────────────────────────────────────
    ImVec4 textPrimary          = ImVec4(0.880f, 0.880f, 0.880f, 1.00f); // #E0E0E0
    ImVec4 textSecondary        = ImVec4(0.530f, 0.530f, 0.530f, 1.00f); // #878787
    ImVec4 textDisabled         = ImVec4(0.350f, 0.350f, 0.350f, 1.00f); // #595959

    // ── Status (the ONLY non-gray colors in the entire palette) ──────
    ImVec4 statusError          = ImVec4(0.820f, 0.250f, 0.250f, 1.00f); // muted red
    ImVec4 statusWarning        = ImVec4(0.820f, 0.680f, 0.250f, 1.00f); // muted amber
};

void ApplyColors(ImGuiStyle& style, const Palette& palette = Palette());
const Palette& GetPalette();

} // namespace EngineEditor::Theme

#endif // THEME_COLORS_H