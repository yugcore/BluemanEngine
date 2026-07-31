#ifndef THEME_COLORS_H
#define THEME_COLORS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct Palette {
    // ── Background ramp (3-tier surface depth) ────────────────────────
    ImVec4 bgBase               = ImVec4(0.07f, 0.075f, 0.085f, 1.00f); // #121316 - Darkest canvas / child base
    ImVec4 bgPanel              = ImVec4(0.11f, 0.115f, 0.135f, 1.00f); // #1C1D22 - Mid-gray panel surface (WindowBg)
    ImVec4 bgHeader             = ImVec4(0.16f, 0.170f, 0.200f, 1.00f); // #292B33 - Input fields, headers (FrameBg)
    ImVec4 bgElevated           = ImVec4(0.22f, 0.230f, 0.270f, 1.00f); // #383B45 - Interactive hover, popups (FrameBgHovered)

    // ── Border ────────────────────────────────────────────────────────
    ImVec4 borderSubtle         = ImVec4(0.25f, 0.270f, 0.320f, 0.65f); // #404552 @ 65%

    // ── Custom accent (Electric Indigo) ───────────────────────────────
    ImVec4 accent               = ImVec4(0.49f, 0.420f, 1.000f, 1.00f); // #7D6BFF - Electric Indigo
    ImVec4 accentHover          = ImVec4(0.58f, 0.520f, 1.000f, 1.00f); // #9485FF
    ImVec4 accentActive         = ImVec4(0.40f, 0.330f, 0.900f, 1.00f); // #6654E6

    // ── Text hierarchy ────────────────────────────────────────────────
    ImVec4 textPrimary          = ImVec4(0.93f, 0.940f, 0.960f, 1.00f); // #EDEFF5 - Crisp, high contrast
    ImVec4 textSecondary        = ImVec4(0.62f, 0.650f, 0.720f, 1.00f); // #9EA6B8 - Muted body text
    ImVec4 textDisabled         = ImVec4(0.42f, 0.450f, 0.520f, 1.00f); // #6B7385

    // ── Status indicators ─────────────────────────────────────────────
    ImVec4 statusError          = ImVec4(0.95f, 0.280f, 0.280f, 1.00f); // Bright crimson red
    ImVec4 statusWarning        = ImVec4(0.96f, 0.660f, 0.160f, 1.00f); // Bright amber
};

void ApplyColors(ImGuiStyle& style, const Palette& palette = Palette());
const Palette& GetPalette();

} // namespace EngineEditor::Theme

#endif // THEME_COLORS_H