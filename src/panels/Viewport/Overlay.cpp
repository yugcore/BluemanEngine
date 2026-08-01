#include "Overlay.h"
#include "core/EditorState.h"
#include "third_party/IconsFontAwesome6.h"
#include <cmath>
#include <algorithm>

namespace EngineEditor::Panels {

static bool WorldToScreen(const float pos[3], const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail, ImVec2& outScreen) {
    float x = pos[0], y = pos[1], z = pos[2];

    // Clip space coordinates: clipPos = pos * view * proj
    float clipX = x * view[0] + y * view[4] + z * view[8] + view[12];
    float clipY = x * view[1] + y * view[5] + z * view[9] + view[13];
    float clipZ = x * view[2] + y * view[6] + z * view[10] + view[14];
    float clipW = x * view[3] + y * view[7] + z * view[11] + view[15];

    float ndcX = clipX * proj[0] + clipY * proj[4] + clipZ * proj[8] + proj[12];
    float ndcY = clipX * proj[1] + clipY * proj[5] + clipZ * proj[9] + proj[13];
    float ndcW = clipX * proj[3] + clipY * proj[7] + clipZ * proj[11] + proj[15];

    if (ndcW <= 0.001f) return false;

    float ndc2X = ndcX / ndcW;
    float ndc2Y = ndcY / ndcW;

    outScreen.x = cursorPos.x + (ndc2X * 0.5f + 0.5f) * viewportAvail.x;
    outScreen.y = cursorPos.y + (1.0f - (ndc2Y * 0.5f + 0.5f)) * viewportAvail.y;

    return true;
}

void RenderViewport3DOverlays(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, int showFlags) {
    if (!drawList || viewportAvail.x <= 0.0f || viewportAvail.y <= 0.0f) return;

    float viewMat[16];
    float projMat[16];

    const auto& camera = EditorState::Get().camera;
    camera.GetViewMatrix(viewMat);
    float aspect = viewportAvail.x / viewportAvail.y;
    camera.GetProjectionMatrix(aspect, projMat);

    bool showGrid = (showFlags & 1) != 0;
    bool showLights = (showFlags & 2) != 0;

    // 1. Render 3D Floor Grid on ground plane (Y = 0)
    if (showGrid) {
        const int gridRadius = 10; // Optimized 20x20 grid cells
        const float gridSpacing = 2.0f;

        ImU32 axisColorX = IM_COL32(235, 65, 65, 220);  // X Axis (Red)
        ImU32 axisColorZ = IM_COL32(65, 135, 245, 220); // Z Axis (Blue)
        ImU32 gridColor  = IM_COL32(180, 195, 210, 80); // Soft grid lines
        ImU32 mainGridColor = IM_COL32(210, 225, 240, 140);

        for (int i = -gridRadius; i <= gridRadius; ++i) {
            float coord = (float)i * gridSpacing;

            // Lines parallel to Z axis
            float p1[3] = { coord, 0.0f, -(float)gridRadius * gridSpacing };
            float p2[3] = { coord, 0.0f,  (float)gridRadius * gridSpacing };
            ImVec2 s1, s2;

            if (WorldToScreen(p1, viewMat, projMat, cursorPos, viewportAvail, s1) &&
                WorldToScreen(p2, viewMat, projMat, cursorPos, viewportAvail, s2)) {
                ImU32 col = (i == 0) ? axisColorZ : ((i % 5 == 0) ? mainGridColor : gridColor);
                float thickness = (i == 0) ? 2.5f : ((i % 5 == 0) ? 1.5f : 1.0f);
                drawList->AddLine(s1, s2, col, thickness);
            }

            // Lines parallel to X axis
            float p3[3] = { -(float)gridRadius * gridSpacing, 0.0f, coord };
            float p4[3] = {  (float)gridRadius * gridSpacing, 0.0f, coord };
            ImVec2 s3, s4;

            if (WorldToScreen(p3, viewMat, projMat, cursorPos, viewportAvail, s3) &&
                WorldToScreen(p4, viewMat, projMat, cursorPos, viewportAvail, s4)) {
                ImU32 col = (i == 0) ? axisColorX : ((i % 5 == 0) ? mainGridColor : gridColor);
                float thickness = (i == 0) ? 2.5f : ((i % 5 == 0) ? 1.5f : 1.0f);
                drawList->AddLine(s3, s4, col, thickness);
            }
        }
    }

    // 2. Render Directional Sun Light Visual Icon & Light Ray Direction Vector
    if (showLights) {
        float sunPos[3] = { 8.0f, 15.0f, -8.0f };
        float sunDir[3] = { -0.5f, -0.8f, -0.3f };
        float sunEnd[3] = { sunPos[0] + sunDir[0] * 6.0f, sunPos[1] + sunDir[1] * 6.0f, sunPos[2] + sunDir[2] * 6.0f };

        ImVec2 sunScr, endScr;
        if (WorldToScreen(sunPos, viewMat, projMat, cursorPos, viewportAvail, sunScr)) {
            // Draw Sun Disk Icon
            drawList->AddCircleFilled(sunScr, 14.0f, IM_COL32(255, 215, 60, 240));
            drawList->AddCircle(sunScr, 18.0f, IM_COL32(255, 235, 130, 200), 0, 2.0f);

            // Draw Sunlight Direction Vector Ray
            if (WorldToScreen(sunEnd, viewMat, projMat, cursorPos, viewportAvail, endScr)) {
                drawList->AddLine(sunScr, endScr, IM_COL32(255, 230, 100, 220), 2.5f);
                drawList->AddCircleFilled(endScr, 4.0f, IM_COL32(255, 230, 100, 255));
            }
        }
    }
}

void RenderViewportStatsHUD(ImVec2 /*cursorPos*/) {
}

} // namespace EngineEditor::Panels
