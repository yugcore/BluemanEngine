#ifndef VIEWPORT_GIZMOS_H
#define VIEWPORT_GIZMOS_H

#include <imgui.h>

namespace EngineEditor::Panels {

void RenderViewportGizmos(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail);

} // namespace EngineEditor::Panels

#endif // VIEWPORT_GIZMOS_H
