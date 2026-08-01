#ifndef VIEWPORT_SELECTION_H
#define VIEWPORT_SELECTION_H

#include <vector>
#include <string>
#include <imgui.h>
#include "ViewportMath.h"

namespace EngineEditor::Panels {

class ViewportSelection {
public:
    static ViewportSelection& Get();

    void SelectSingle(const std::string& nodeName, bool add = false, bool toggle = false);
    void SelectAll();
    void InvertSelection();
    void ClearSelection();

    void SelectParent();
    void SelectChildren();

    // Marquee Drag Selection
    void StartMarquee(ImVec2 startPos);
    void UpdateMarquee(ImVec2 currentPos);
    void EndMarquee(ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], bool shiftHeld, bool ctrlHeld);
    void RenderMarquee(ImDrawList* drawList);

    bool IsMarqueeActive() const { return m_IsMarqueeActive; }

private:
    bool m_IsMarqueeActive = false;
    ImVec2 m_MarqueeStart{ 0.0f, 0.0f };
    ImVec2 m_MarqueeCurrent{ 0.0f, 0.0f };
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_SELECTION_H
