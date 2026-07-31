#include "Gizmos.h"
#include "core/EditorState.h"
#include "third_party/ImGuizmo/ImGuizmo.h"

namespace EngineEditor::Panels {

void RenderViewportGizmos(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail) {
    const std::string& selectedNode = EditorState::Get().selectedNodeName;
    if (!selectedNode.empty()) {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetDrawlist(drawList);
        ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportAvail.x, viewportAvail.y);

        float viewMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, -5.0f, 1.0f
        };
        float projMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };

        float modelMatrix[16];
        auto& transform = EditorState::Get().activeTransform;
        ImGuizmo::RecomposeMatrixFromComponents(transform.location, transform.rotation, transform.scale, modelMatrix);

        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        if (EditorState::Get().gizmoOp == GizmoOperation::Rotate) op = ImGuizmo::ROTATE;
        else if (EditorState::Get().gizmoOp == GizmoOperation::Scale) op = ImGuizmo::SCALE;

        ImGuizmo::Manipulate(viewMatrix, projMatrix, op, ImGuizmo::WORLD, modelMatrix);
        ImGuizmo::DecomposeMatrixToComponents(modelMatrix, transform.location, transform.rotation, transform.scale);

        // UE5 Corner-Handle Bracket Selection Outline
        ImVec2 center = ImVec2(cursorPos.x + viewportAvail.x * 0.5f, cursorPos.y + viewportAvail.y * 0.5f);
        float boxW = 120.0f, boxH = 100.0f;
        float x0 = center.x - boxW * 0.5f;
        float y0 = center.y - boxH * 0.5f;
        float x1 = center.x + boxW * 0.5f;
        float y1 = center.y + boxH * 0.5f;
        float cornerLen = 14.0f;

        ImU32 selColor = IM_COL32(242, 169, 59, 240); // #F2A93B Amber

        // Top-Left Corner
        drawList->AddLine(ImVec2(x0, y0), ImVec2(x0 + cornerLen, y0), selColor, 2.0f);
        drawList->AddLine(ImVec2(x0, y0), ImVec2(x0, y0 + cornerLen), selColor, 2.0f);

        // Top-Right Corner
        drawList->AddLine(ImVec2(x1, y0), ImVec2(x1 - cornerLen, y0), selColor, 2.0f);
        drawList->AddLine(ImVec2(x1, y0), ImVec2(x1, y0 + cornerLen), selColor, 2.0f);

        // Bottom-Left Corner
        drawList->AddLine(ImVec2(x0, y1), ImVec2(x0 + cornerLen, y1), selColor, 2.0f);
        drawList->AddLine(ImVec2(x0, y1), ImVec2(x0, y1 - cornerLen), selColor, 2.0f);

        // Bottom-Right Corner
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x1 - cornerLen, y1), selColor, 2.0f);
        drawList->AddLine(ImVec2(x1, y1), ImVec2(x1, y1 - cornerLen), selColor, 2.0f);

        // XYZ Axis Indicators (Red=X, Green=Y, Blue=Z)
        drawList->AddLine(center, ImVec2(center.x + 35.0f, center.y), IM_COL32(235, 65, 65, 255), 2.5f);
        drawList->AddText(ImVec2(center.x + 38.0f, center.y - 6.0f), IM_COL32(235, 65, 65, 255), "X");

        drawList->AddLine(center, ImVec2(center.x, center.y - 35.0f), IM_COL32(65, 215, 65, 255), 2.5f);
        drawList->AddText(ImVec2(center.x - 4.0f, center.y - 50.0f), IM_COL32(65, 215, 65, 255), "Y");

        drawList->AddLine(center, ImVec2(center.x - 22.0f, center.y + 22.0f), IM_COL32(65, 135, 245, 255), 2.5f);
        drawList->AddText(ImVec2(center.x - 34.0f, center.y + 22.0f), IM_COL32(65, 135, 245, 255), "Z");
    }

    // ViewManipulate Orientation Cube
    float viewMat[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    ImGuizmo::ViewManipulate(viewMat, 8.0f, ImVec2(cursorPos.x + viewportAvail.x - 84.0f, cursorPos.y + 45.0f), ImVec2(60.0f, 60.0f), IM_COL32(23, 23, 26, 200));
}

} // namespace EngineEditor::Panels
