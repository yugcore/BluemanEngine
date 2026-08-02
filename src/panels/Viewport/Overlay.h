#ifndef VIEWPORT_OVERLAY_H
#define VIEWPORT_OVERLAY_H

#include <imgui.h>

namespace EngineEditor::Panels {

void RenderViewport3DOverlays(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, int showFlags);

} // namespace EngineEditor::Panels

#endif // VIEWPORT_OVERLAY_H
