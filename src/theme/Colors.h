#ifndef THEME_COLORS_H
#define THEME_COLORS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct Palette {
    // ── Background ramp (Monochrome dark depth) ────────────────────────
    ImVec4 bgBase               = ImVec4(0.08f, 0.08f, 0.08f, 1.00f); // #141414 - Darkest neutral canvas
    ImVec4 bgPanel              = ImVec4(0.12f, 0.12f, 0.12f, 1.00f); // #1F1F1F - Dark gray panel surface
    ImVec4 bgHeader             = ImVec4(0.18f, 0.18f, 0.18f, 1.00f); // #2E2E2E - Input fields, headers
    ImVec4 bgElevated           = ImVec4(0.25f, 0.25f, 0.25f, 1.00f); // #404040 - Hover, popups

    // ── Border ────────────────────────────────────────────────────────
    ImVec4 borderSubtle         = ImVec4(0.30f, 0.30f, 0.30f, 0.65f); // #4D4D4D @ 65%

    // ── Custom accent (Monochrome Crisp White & Silver) ───────────────
    ImVec4 accent               = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // Pure Crisp White
    ImVec4 accentHover          = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Brightest White
    ImVec4 accentActive         = ImVec4(0.80f, 0.80f, 0.80f, 1.00f); // Silver

    // ── Text hierarchy ────────────────────────────────────────────────
    ImVec4 textPrimary          = ImVec4(0.95f, 0.95f, 0.95f, 1.00f); // #F2F2F2 - Crisp, high contrast
    ImVec4 textSecondary        = ImVec4(0.65f, 0.65f, 0.65f, 1.00f); // #A6A6A6 - Muted body text
    ImVec4 textDisabled         = ImVec4(0.45f, 0.45f, 0.45f, 1.00f); // #737373

    // ── Status indicators ─────────────────────────────────────────────
    ImVec4 statusError          = ImVec4(0.95f, 0.28f, 0.28f, 1.00f); // Crimson red
    ImVec4 statusWarning        = ImVec4(0.96f, 0.66f, 0.16f, 1.00f); // Amber
};

void ApplyColors(ImGuiStyle& style, const Palette& palette = Palette());
const Palette& GetPalette();

} // namespace EngineEditor::Theme

#endif // THEME_COLORS_H