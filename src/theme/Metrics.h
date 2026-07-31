#ifndef THEME_METRICS_H
#define THEME_METRICS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct Metrics {
    // ── Base spacing unit: 4 px ───────────────────────────────────────
    //    Every value below is a multiple of 4.
    static constexpr float unit = 4.0f;

    // ── Uniform corner radius ─────────────────────────────────────────
    float windowRounding    = 2.0f;   // 0.5 × unit
    float childRounding     = 2.0f;
    float frameRounding     = 2.0f;
    float popupRounding     = 2.0f;
    float scrollbarRounding = 2.0f;
    float grabRounding      = 2.0f;
    float tabRounding       = 2.0f;

    // ── Border sizes (subtle 1 px borders) ────────────────────────────
    float windowBorderSize  = 1.0f;
    float childBorderSize   = 0.0f;   // prefer bg-step contrast
    float popupBorderSize   = 1.0f;
    float frameBorderSize   = 0.0f;
    float tabBorderSize     = 0.0f;

    // ── Spacing (multiples of 4) ──────────────────────────────────────
    ImVec2 windowPadding    = ImVec2(8.0f, 8.0f);     // 2 × unit
    ImVec2 framePadding     = ImVec2(8.0f, 4.0f);     // 2u × 1u
    ImVec2 itemSpacing      = ImVec2(8.0f, 4.0f);     // 2u × 1u
    ImVec2 itemInnerSpacing = ImVec2(4.0f, 4.0f);     // 1u × 1u
    float indentSpacing     = 16.0f;                   // 4 × unit
    float scrollbarSize     = 12.0f;                   // 3 × unit

    // ── Panel-level grid constants (shared across all panels) ─────────
    static constexpr float panelLeftMargin    = 8.0f;  // 2 × unit
    static constexpr float labelColumnWidth   = 100.0f; // 25 × unit (label col)
    static constexpr float rowHeight          = 24.0f; // 6 × unit
    static constexpr float headerHeight       = 28.0f; // 7 × unit
    static constexpr float groupGap           = 12.0f; // 3 × unit (between groups)
    static constexpr float intraGroupGap      = 4.0f;  // 1 × unit (within group)
    static constexpr float sectionIndent      = 8.0f;  // 2 × unit
};

void ApplyMetrics(ImGuiStyle& style, const Metrics& metrics = Metrics());

} // namespace EngineEditor::Theme

#endif // THEME_METRICS_H