#ifndef LAYOUT_DOCKSPACE_H
#define LAYOUT_DOCKSPACE_H

#include <imgui.h>

namespace EngineEditor::Layout {

struct DockspaceBounds {
    ImVec2 pos;
    ImVec2 size;
};

DockspaceBounds CalculateDockspaceBounds(float topOffset = 72.0f, float bottomOffset = 32.0f);
ImGuiID RenderDockspaceHost(const DockspaceBounds& bounds, ImGuiDockNodeFlags flags = ImGuiDockNodeFlags_PassthruCentralNode);

} // namespace EngineEditor::Layout

#endif // LAYOUT_DOCKSPACE_H
