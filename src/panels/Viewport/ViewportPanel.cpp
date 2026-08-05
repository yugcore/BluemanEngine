#include "ViewportPanel.h"
#include "ViewportMath.h"
#include "ViewportPicker.h"
#include "ViewportSelection.h"
#include "ViewportMeasurement.h"
#include "ViewportContextMenu.h"
#include "ViewportPreferences.h"
#include "ViewportDragDrop.h"
#include "editor_overlay.h"
#include "render/ViewportRenderer.h"
#include "render/ZeGFXAdapter.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include "core/CommandStack.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <cstdio>
#include <filesystem>


namespace EngineEditor {

static int s_PerspectiveIdx = 0;
static int s_QualityIdx = 0;
static int s_ShowFlags = 7;

static void RenderStatusPill(const char* label, const ImVec4& color) {
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(28, 34, 44, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(40, 48, 60, 255));
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));

    ImGui::Button(label);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void RenderViewportPanel(bool* pOpen) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (!ImGui::Begin("Viewport", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImVec2 viewportAvail = ImGui::GetContentRegionAvail();
    uint32_t width = (uint32_t)viewportAvail.x;
    uint32_t height = (uint32_t)viewportAvail.y;

    if (width > 0 && height > 0) {
        ViewportRenderer::Get().Resize(width, height);
    }

    bool isHovered = ImGui::IsWindowHovered();
    bool isFocused = ImGui::IsWindowFocused();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    float deltaTime = io.DeltaTime;

    if (isHovered && (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Middle))) {
        ImGui::SetWindowFocus("Viewport");
        if (ImGui::GetActiveID() != 0) {
            ImGui::ClearActiveID();
        }
    }

    auto& camera = EditorState::Get().camera;

    // 1. Build Raw Input + Hotkeys + Drag-Drop Payload Event for Layer 1 Overlay
    zegfx::overlay::ViewportInputEvent overlayEvt = {};
    overlayEvt.mouseLocalX = io.MousePos.x - cursorPos.x;
    overlayEvt.mouseLocalY = io.MousePos.y - cursorPos.y;
    overlayEvt.mouseDeltaX = io.MouseDelta.x;
    overlayEvt.mouseDeltaY = io.MouseDelta.y;
    overlayEvt.scrollDelta = io.MouseWheel;
    overlayEvt.viewportWidth = viewportAvail.x;
    overlayEvt.viewportHeight = viewportAvail.y;
    overlayEvt.isHovered = isHovered;
    overlayEvt.isFocused = isFocused;
    overlayEvt.modifiers.alt = io.KeyAlt;
    overlayEvt.modifiers.ctrl = io.KeyCtrl;
    overlayEvt.modifiers.shift = io.KeyShift;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left))   overlayEvt.mouseButtonsDown |= (uint8_t)zegfx::overlay::MouseButton::Left;
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right))  overlayEvt.mouseButtonsDown |= (uint8_t)zegfx::overlay::MouseButton::Right;
    if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) overlayEvt.mouseButtonsDown |= (uint8_t)zegfx::overlay::MouseButton::Middle;

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))   overlayEvt.mouseButtonsPressed |= (uint8_t)zegfx::overlay::MouseButton::Left;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))  overlayEvt.mouseButtonsPressed |= (uint8_t)zegfx::overlay::MouseButton::Right;
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) overlayEvt.mouseButtonsPressed |= (uint8_t)zegfx::overlay::MouseButton::Middle;

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))   overlayEvt.mouseButtonsReleased |= (uint8_t)zegfx::overlay::MouseButton::Left;
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))  overlayEvt.mouseButtonsReleased |= (uint8_t)zegfx::overlay::MouseButton::Right;
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Middle)) overlayEvt.mouseButtonsReleased |= (uint8_t)zegfx::overlay::MouseButton::Middle;

    // Forward Raw Hotkeys State
    if (camera.GetMode() == CameraMode::Idle && (isHovered || isFocused) && !io.WantTextInput) {
        overlayEvt.keyW = ImGui::IsKeyPressed(ImGuiKey_W);
        overlayEvt.keyE = ImGui::IsKeyPressed(ImGuiKey_E);
        overlayEvt.keyR = ImGui::IsKeyPressed(ImGuiKey_R);
        overlayEvt.keyF = ImGui::IsKeyPressed(ImGuiKey_F);
        overlayEvt.keyHome = ImGui::IsKeyPressed(ImGuiKey_Home);
        overlayEvt.keyInsert = ImGui::IsKeyPressed(ImGuiKey_Insert);
        overlayEvt.keyD = ImGui::IsKeyPressed(ImGuiKey_D);
        overlayEvt.keySpace = ImGui::IsKeyPressed(ImGuiKey_Space);
        overlayEvt.keyA = ImGui::IsKeyPressed(ImGuiKey_A);
        overlayEvt.keyI = ImGui::IsKeyPressed(ImGuiKey_I);
        overlayEvt.keyZ = ImGui::IsKeyPressed(ImGuiKey_Z);
        overlayEvt.keyY = ImGui::IsKeyPressed(ImGuiKey_Y);
        overlayEvt.keyH = ImGui::IsKeyPressed(ImGuiKey_H);
        overlayEvt.keyV = ImGui::IsKeyDown(ImGuiKey_V);
        overlayEvt.keyEscape = ImGui::IsKeyPressed(ImGuiKey_Escape);

        for (int bIdx = 0; bIdx < 10; ++bIdx) {
            ImGuiKey numKey = (ImGuiKey)(ImGuiKey_0 + bIdx);
            if (ImGui::IsKeyPressed(numKey)) {
                overlayEvt.keyNumPressed = bIdx;
                break;
            }
        }
    }

    // Forward Drag & Drop Payload State
    if (const ImGuiPayload* payload = ImGui::GetDragDropPayload()) {
        if (payload->IsDataType("CONTENT_BROWSER_ASSET_PATH") && payload->Data) {
            overlayEvt.hasDragDropPayload = true;
            overlayEvt.dragDropAssetPath = (const char*)payload->Data;
            if (ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET_PATH")) {
                overlayEvt.isDragDropDropped = true;
            }
        }
    }

    auto& overlay = ViewportRenderer::Get().GetEditorOverlay();
    overlay.UpdateInput(overlayEvt);

    // 2. Build ViewportInputState for Camera Update
    ViewportInputState inputState = {};
    inputState.isHovered = isHovered;
    inputState.isFocused = isFocused;
    inputState.rmbDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
    inputState.lmbDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
    inputState.mmbDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);
    inputState.altHeld = io.KeyAlt;
    inputState.shiftHeld = io.KeyShift;
    inputState.ctrlHeld = io.KeyCtrl;
    inputState.mouseDeltaX = io.MouseDelta.x;
    inputState.mouseDeltaY = io.MouseDelta.y;
    inputState.scrollDelta = io.MouseWheel;

    inputState.keyW = ImGui::IsKeyDown(ImGuiKey_W);
    inputState.keyA = ImGui::IsKeyDown(ImGuiKey_A);
    inputState.keyS = ImGui::IsKeyDown(ImGuiKey_S);
    inputState.keyD = ImGui::IsKeyDown(ImGuiKey_D);
    inputState.keyQ = ImGui::IsKeyDown(ImGuiKey_Q);
    inputState.keyE = ImGui::IsKeyDown(ImGuiKey_E);

    // Gate camera interactions if gizmo drag is active
    if (overlay.IsDraggingGizmo()) {
        inputState.lmbDown = false;
        inputState.rmbDown = false;
        inputState.mmbDown = false;
    }

    camera.Update(deltaTime, inputState);

    // 3. Render 3D Scene via ZeGFX Engine
    ViewportRenderer::Get().RenderScene(deltaTime);

    // 4. Render Finished Viewport Texture
    uint64_t textureID = ViewportRenderer::Get().GetTextureID();
    if (textureID == 0) {
        ImGui::Dummy(viewportAvail);
    } else {
        ImGui::Image((ImTextureID)textureID, viewportAvail, ImVec2(0, 1), ImVec2(1, 0));
    }

    // --- ImGuizmo 3D Transform Gizmo ---
    std::string selectedNode = EditorState::Get().selectedNodeName;
    SceneNode* sNode = SceneGraph::Get().FindNodeMutable(selectedNode);

    if (sNode) {
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

        ImVec2 cursorPos = ImGui::GetItemRectMin();
        ImVec2 itemSize = ImGui::GetItemRectSize();
        ImGuizmo::SetRect(cursorPos.x, cursorPos.y, itemSize.x, itemSize.y);

        float viewMatrix[16];
        float projMatrix[16];
        camera.GetViewMatrix(viewMatrix);
        float aspect = (itemSize.y > 0.0f) ? (itemSize.x / itemSize.y) : 1.777f;
        camera.GetProjectionMatrix(aspect, projMatrix);

        float modelMatrix[16];
        float location[3] = { sNode->location[0], sNode->location[1], sNode->location[2] };
        float rotation[3] = { sNode->rotation[0], sNode->rotation[1], sNode->rotation[2] };
        float scale[3]    = { sNode->scale[0],    sNode->scale[1],    sNode->scale[2] };

        ImGuizmo::RecomposeMatrixFromComponents(location, rotation, scale, modelMatrix);

        ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
        if (EditorState::Get().gizmoOp == GizmoOperation::Rotate) op = ImGuizmo::ROTATE;
        else if (EditorState::Get().gizmoOp == GizmoOperation::Scale) op = ImGuizmo::SCALE;

        ImGuizmo::MODE mode = (op == ImGuizmo::SCALE) ? ImGuizmo::LOCAL :
            (EditorState::Get().activeTransformSpace == TransformSpace::World ? ImGuizmo::WORLD : ImGuizmo::LOCAL);

        if (camera.GetMode() == CameraMode::Idle) {
            ImGuizmo::Enable(true);
            if (ImGuizmo::Manipulate(viewMatrix, projMatrix, op, mode, modelMatrix)) {
                ImGuizmo::DecomposeMatrixToComponents(modelMatrix, location, rotation, scale);
                sNode->location[0] = location[0]; sNode->location[1] = location[1]; sNode->location[2] = location[2];
                sNode->rotation[0] = rotation[0]; sNode->rotation[1] = rotation[1]; sNode->rotation[2] = rotation[2];
                sNode->scale[0]    = scale[0];    sNode->scale[1]    = scale[1];    sNode->scale[2]    = scale[2];
                SceneGraph::Get().SyncNodeComponents(*sNode);
            }
        }
    }

    // 5. Host Viewport Context Menu if Overlay Pick Result Is Pending
    if (overlay.HasPendingRightClickPick()) {
        if (!overlay.GetPendingPickNodeName().empty() && camera.GetMode() == CameraMode::Idle) {
            Panels::ViewportContextMenu::Get().OpenMenu(overlay.GetPendingPickNodeName());
        }
        overlay.ClearRightClickPick();
    }
    Panels::ViewportContextMenu::Get().Render();

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace EngineEditor
