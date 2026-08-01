#ifndef VIEWPORT_DRAG_DROP_H
#define VIEWPORT_DRAG_DROP_H

#include <imgui.h>
#include <string>
#include "ViewportPicker.h"

namespace EngineEditor::Panels {

class ViewportDragDrop {
public:
    static ViewportDragDrop& Get();

    void HandleDragDropTarget(ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]);
    void RenderGhostPreview(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]);

private:
    bool m_IsDraggingAsset = false;
    std::string m_DraggedAssetPath;
    RaycastHit m_CurrentHit;
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_DRAG_DROP_H
