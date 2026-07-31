#include "Metrics.h"

namespace EngineEditor::Theme {

void ApplyMetrics(ImGuiStyle& style, const Metrics& metrics) {
    style.WindowRounding    = metrics.windowRounding;
    style.ChildRounding     = metrics.childRounding;
    style.FrameRounding     = metrics.frameRounding;
    style.PopupRounding     = metrics.popupRounding;
    style.ScrollbarRounding = metrics.scrollbarRounding;
    style.GrabRounding      = metrics.grabRounding;
    style.TabRounding       = metrics.tabRounding;

    style.WindowBorderSize  = metrics.windowBorderSize;
    style.ChildBorderSize   = metrics.childBorderSize;
    style.PopupBorderSize   = metrics.popupBorderSize;
    style.FrameBorderSize   = metrics.frameBorderSize;
    style.TabBorderSize     = metrics.tabBorderSize;

    style.WindowPadding     = metrics.windowPadding;
    style.FramePadding      = metrics.framePadding;
    style.ItemSpacing       = metrics.itemSpacing;
    style.ItemInnerSpacing  = metrics.itemInnerSpacing;
    style.IndentSpacing     = metrics.indentSpacing;
    style.ScrollbarSize     = metrics.scrollbarSize;
}

} // namespace EngineEditor::Theme
