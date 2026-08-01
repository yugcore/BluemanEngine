#ifndef VIEWPORT_MEASUREMENT_H
#define VIEWPORT_MEASUREMENT_H

#include <imgui.h>
#include "ViewportMath.h"

namespace EngineEditor::Panels {

class ViewportMeasurement {
public:
    static ViewportMeasurement& Get();

    void Activate();
    void Deactivate();
    bool IsActive() const { return m_IsActive; }

    void OnMouseClick(const Vec3f& worldPos);
    void OnMouseMove(const Vec3f& worldPos);

    void Render(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]);

private:
    bool m_IsActive = false;
    bool m_IsDragging = false;
    Vec3f m_StartPos{ 0.0f, 0.0f, 0.0f };
    Vec3f m_EndPos{ 0.0f, 0.0f, 0.0f };
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_MEASUREMENT_H
