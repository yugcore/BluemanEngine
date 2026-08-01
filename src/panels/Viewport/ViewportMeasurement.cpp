#include "ViewportMeasurement.h"
#include <cstdio>
#include <cmath>

namespace EngineEditor::Panels {

ViewportMeasurement& ViewportMeasurement::Get() {
    static ViewportMeasurement instance;
    return instance;
}

void ViewportMeasurement::Activate() {
    m_IsActive = true;
    m_IsDragging = false;
}

void ViewportMeasurement::Deactivate() {
    m_IsActive = false;
    m_IsDragging = false;
}

void ViewportMeasurement::OnMouseClick(const Vec3f& worldPos) {
    if (!m_IsActive) return;
    if (!m_IsDragging) {
        m_StartPos = worldPos;
        m_EndPos = worldPos;
        m_IsDragging = true;
    } else {
        m_EndPos = worldPos;
        m_IsDragging = false;
    }
}

void ViewportMeasurement::OnMouseMove(const Vec3f& worldPos) {
    if (m_IsActive && m_IsDragging) {
        m_EndPos = worldPos;
    }
}

void ViewportMeasurement::Render(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]) {
    if (!m_IsActive || !drawList) return;

    ImVec2 sStart, sEnd;
    bool vStart = ViewportMath::WorldToScreen(m_StartPos, view, proj, cursorPos, viewportAvail, sStart);
    bool vEnd   = ViewportMath::WorldToScreen(m_EndPos, view, proj, cursorPos, viewportAvail, sEnd);

    if (vStart && vEnd) {
        // Draw measurement line
        drawList->AddLine(sStart, sEnd, IM_COL32(255, 200, 50, 255), 2.5f);
        drawList->AddCircleFilled(sStart, 6.0f, IM_COL32(255, 220, 0, 255));
        drawList->AddCircleFilled(sEnd, 6.0f, IM_COL32(255, 100, 50, 255));

        // Calculate 3D Euclidean distance & vector components
        float dist = ViewportMath::Distance(m_StartPos, m_EndPos);
        Vec3f delta = ViewportMath::Sub(m_EndPos, m_StartPos);

        char buf[128];
        snprintf(buf, sizeof(buf), "Dist: %.2fm | dx: %.2f | dy: %.2f | dz: %.2f", dist, delta.x, delta.y, delta.z);

        ImVec2 midScreen((sStart.x + sEnd.x) * 0.5f, (sStart.y + sEnd.y) * 0.5f);

        // Render background pill & distance text
        ImVec2 textSize = ImGui::CalcTextSize(buf);
        drawList->AddRectFilled(ImVec2(midScreen.x - 6.0f, midScreen.y - 14.0f), ImVec2(midScreen.x + textSize.x + 10.0f, midScreen.y + 12.0f), IM_COL32(15, 20, 28, 230), 4.0f);
        drawList->AddText(ImVec2(midScreen.x, midScreen.y - 10.0f), IM_COL32(255, 220, 80, 255), buf);
    }
}

} // namespace EngineEditor::Panels
