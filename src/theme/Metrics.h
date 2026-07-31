#ifndef THEME_METRICS_H
#define THEME_METRICS_H

#include <imgui.h>

namespace EngineEditor::Theme {

struct Metrics {
    // ── Base spacing unit: 8 px ───────────────────────────────────────
    static constexpr float unit = 8.0f;

    // ── Corner rounding (0.0f everywhere for clean flat technical identity) ──
    float windowRounding    = 0.0f;   // Completely flat windows
    float childRounding     = 0.0f;   // Completely flat child panels
    float frameRounding     = 0.0f;   // Completely flat input fields / buttons
    float popupRounding     = 0.0f;   // Completely flat popups & dropdowns
    float scrollbarRounding = 0.0f;   // Completely flat scrollbars
    float grabRounding      = 0.0f;   // Completely flat sliders & grabs
    float tabRounding       = 0.0f;   // Completely flat tabs

    // ── Borders ───────────────────────────────────────────────────────
    float windowBorderSize  = 1.0f;
    float childBorderSize   = 1.0f;
    float popupBorderSize   = 1.0f;
    float frameBorderSize   = 1.0f;
    float tabBorderSize     = 0.0f;

    // ── Spacing (exact prompt specifications) ─────────────────────────
    ImVec2 windowPadding    = ImVec2(12.0f, 12.0f);   // 12x12 window padding
    ImVec2 framePadding     = ImVec2(10.0f, 6.0f);    // 10x6 frame padding
    ImVec2 itemSpacing      = ImVec2(10.0f, 8.0f);    // 10x8 item spacing
    ImVec2 itemInnerSpacing = ImVec2(8.0f, 8.0f);
    float indentSpacing     = 24.0f;
    float scrollbarSize     = 14.0f;

    // ── Shared panel grid constants (8px grid scale) ──────────────────
    static constexpr float panelLeftMargin    = 16.0f; // 2u
    static constexpr float labelColumnWidth   = 150.0f;
    static constexpr float rowHeight          = 32.0f; // 4u
    static constexpr float headerHeight       = 42.0f; // 12px+ vertical padding header height
    static constexpr float groupGap           = 24.0f; // 3u - Cluster gap between toolbar groups
    static constexpr float intraGroupGap      = 8.0f;  // 1u - Gap within a cluster
    static constexpr float sectionIndent      = 16.0f; // 2u

    // ── Content Browser card grid (Elevated cards with 8px margin) ─────
    static constexpr float tileWidth          = 124.0f; // Card width
    static constexpr float tileHeight         = 144.0f; // Card height with label & margin
    static constexpr float tileSize           = 124.0f;
    static constexpr float tileGap            = 16.0f;  // 2u grid gap

    // ── Outliner / multi-column table widths ──────────────────────────
    static constexpr float outlinerNameWidth  = 260.0f;
    static constexpr float outlinerWorldWidth = 130.0f;
    static constexpr float outlinerPanelWidth = 120.0f;
};

void ApplyMetrics(ImGuiStyle& style, const Metrics& metrics = Metrics());

} // namespace EngineEditor::Theme

#endif // THEME_METRICS_H