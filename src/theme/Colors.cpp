#include "Colors.h"

namespace EngineEditor::Theme {

static Palette s_Palette;

void ApplyColors(ImGuiStyle& style, const Palette& palette) {
    s_Palette = palette;
    ImVec4* colors = style.Colors;

    colors[ImGuiCol_Text]                  = palette.textPrimary;
    colors[ImGuiCol_TextDisabled]          = palette.textDisabled;
    colors[ImGuiCol_WindowBg]              = palette.bgPanel;
    colors[ImGuiCol_ChildBg]               = palette.bgBase;
    colors[ImGuiCol_PopupBg]               = palette.bgElevated;
    colors[ImGuiCol_Border]                = palette.borderSubtle;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    // Frame backgrounds (input fields, combo boxes)
    colors[ImGuiCol_FrameBg]               = palette.bgHeader;
    colors[ImGuiCol_FrameBgHovered]        = palette.bgElevated;
    colors[ImGuiCol_FrameBgActive]         = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.35f);

    // Title bars
    colors[ImGuiCol_TitleBg]               = palette.bgBase;
    colors[ImGuiCol_TitleBgActive]         = ImVec4(palette.bgHeader.x + 0.02f, palette.bgHeader.y + 0.02f, palette.bgHeader.z + 0.02f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = palette.bgBase;
    colors[ImGuiCol_MenuBarBg]             = palette.bgBase;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]           = palette.bgBase;
    colors[ImGuiCol_ScrollbarGrab]         = palette.bgElevated;
    colors[ImGuiCol_ScrollbarGrabHovered]  = palette.accentHover;
    colors[ImGuiCol_ScrollbarGrabActive]   = palette.accent;

    // Checks and sliders
    colors[ImGuiCol_CheckMark]             = palette.accent;
    colors[ImGuiCol_SliderGrab]            = palette.accent;
    colors[ImGuiCol_SliderGrabActive]      = palette.accentHover;

    // Buttons
    colors[ImGuiCol_Button]                = palette.bgHeader;
    colors[ImGuiCol_ButtonHovered]         = palette.bgElevated;
    colors[ImGuiCol_ButtonActive]          = palette.accent;

    // Headers (tree nodes, outliner rows, table headers) - Electric Indigo accent
    colors[ImGuiCol_Header]                = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.25f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.40f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.60f);

    // Separators
    colors[ImGuiCol_Separator]             = palette.borderSubtle;
    colors[ImGuiCol_SeparatorHovered]      = palette.accentHover;
    colors[ImGuiCol_SeparatorActive]       = palette.accentActive;

    // Resize grips
    colors[ImGuiCol_ResizeGrip]            = palette.borderSubtle;
    colors[ImGuiCol_ResizeGripHovered]     = palette.accentHover;
    colors[ImGuiCol_ResizeGripActive]      = palette.accentActive;

    // Tabs - Electric Indigo top accent line
    colors[ImGuiCol_Tab]                   = palette.bgBase;
    colors[ImGuiCol_TabHovered]            = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.35f);
    colors[ImGuiCol_TabActive]             = palette.bgPanel;
    colors[ImGuiCol_TabSelectedOverline]   = palette.accent;
    colors[ImGuiCol_TabDimmed]             = palette.bgBase;
    colors[ImGuiCol_TabDimmedSelected]     = palette.bgPanel;
    colors[ImGuiCol_TabDimmedSelectedOverline] = palette.accent;

    // Docking
    colors[ImGuiCol_DockingPreview]        = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.40f);
    colors[ImGuiCol_DockingEmptyBg]        = palette.bgBase;

    // Plots
    colors[ImGuiCol_PlotLines]             = palette.accent;
    colors[ImGuiCol_PlotLinesHovered]      = palette.accentHover;
    colors[ImGuiCol_PlotHistogram]         = palette.accent;
    colors[ImGuiCol_PlotHistogramHovered]  = palette.accentHover;

    // Tables
    colors[ImGuiCol_TableHeaderBg]         = palette.bgHeader;
    colors[ImGuiCol_TableBorderStrong]     = palette.borderSubtle;
    colors[ImGuiCol_TableBorderLight]      = ImVec4(palette.borderSubtle.x, palette.borderSubtle.y, palette.borderSubtle.z, 0.40f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.02f);

    // Selection and navigation
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(palette.accent.x, palette.accent.y, palette.accent.z, 0.35f);
    colors[ImGuiCol_DragDropTarget]        = palette.accent;
    colors[ImGuiCol_NavHighlight]          = palette.accent;
}

const Palette& GetPalette() {
    return s_Palette;
}

} // namespace EngineEditor::Theme