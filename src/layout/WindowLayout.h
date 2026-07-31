#ifndef LAYOUT_WINDOW_LAYOUT_H
#define LAYOUT_WINDOW_LAYOUT_H

#include "Dockspace.h"
#include <imgui.h>

namespace EngineEditor::Layout {

void SetupDefaultLayout(ImGuiID dockspaceId, const DockspaceBounds& bounds, bool forceRebuild = false);

} // namespace EngineEditor::Layout

#endif // LAYOUT_WINDOW_LAYOUT_H
