#include "Gizmos.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/CommandStack.h"
#include "third_party/ImGuizmo/ImGuizmo.h"

namespace EngineEditor::Panels {

static bool s_WasUsingLastFrame = false;
static TransformData s_PreDragTransform = {};
static std::string s_DragNodeName = "";
static uint64_t s_DragNodeId = 0;

void RenderViewportGizmos(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail) {
    std::string selectedNode = EditorState::Get().selectedNodeName;
    if (selectedNode.empty()) {
        s_WasUsingLastFrame = false;
        return;
    }

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(drawList);
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportAvail.x, viewportAvail.y);

    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedNode);
    if (!node) {
        s_WasUsingLastFrame = false;
        return;
    }

    auto& transform = EditorState::Get().activeTransform;

    if (!ImGuizmo::IsUsing()) {
        transform.location[0] = node->location[0];
        transform.location[1] = node->location[1];
        transform.location[2] = node->location[2];

        transform.rotation[0] = node->rotation[0];
        transform.rotation[1] = node->rotation[1];
        transform.rotation[2] = node->rotation[2];

        transform.scale[0] = node->scale[0];
        transform.scale[1] = node->scale[1];
        transform.scale[2] = node->scale[2];
    }

    float viewMatrix[16];
    float projMatrix[16];

    const auto& camera = EditorState::Get().camera;
    camera.GetViewMatrix(viewMatrix);

    float aspectRatio = (viewportAvail.y > 0.0f) ? (viewportAvail.x / viewportAvail.y) : 1.777f;
    camera.GetProjectionMatrix(aspectRatio, projMatrix);

    float modelMatrix[16];
    ImGuizmo::RecomposeMatrixFromComponents(transform.location, transform.rotation, transform.scale, modelMatrix);

    ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
    if (EditorState::Get().gizmoOp == GizmoOperation::Rotate) op = ImGuizmo::ROTATE;
    else if (EditorState::Get().gizmoOp == GizmoOperation::Scale) op = ImGuizmo::SCALE;

    // Force LOCAL mode for scale operations
    ImGuizmo::MODE mode = (op == ImGuizmo::SCALE)
        ? ImGuizmo::LOCAL
        : ((EditorState::Get().activeTransformSpace == TransformSpace::World) ? ImGuizmo::WORLD : ImGuizmo::LOCAL);

    // Dynamic Snap Settings
    const auto& snap = EditorState::Get().snapSettings;
    bool ctrlHeld = ImGui::GetIO().KeyCtrl;
    float snapValues[3] = { 0.0f, 0.0f, 0.0f };
    float* pSnap = nullptr;

    if (op == ImGuizmo::TRANSLATE && (snap.enableTranslate ^ ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.translateSnap;
        pSnap = snapValues;
    } else if (op == ImGuizmo::ROTATE && (snap.enableRotate ^ ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.rotateSnap;
        pSnap = snapValues;
    } else if (op == ImGuizmo::SCALE && (snap.enableScale ^ ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.scaleSnap;
        pSnap = snapValues;
    }

    // Gate gizmo interaction behind camera state (disable if camera is in Fly/Orbit/Pan)
    bool isCameraIdle = (camera.GetMode() == CameraMode::Idle);
    ImGuizmo::Enable(isCameraIdle);

    ImGuizmo::Manipulate(viewMatrix, projMatrix, op, mode, modelMatrix, nullptr, pSnap);

    if (ImGuizmo::IsUsing()) {
        ImGuizmo::DecomposeMatrixToComponents(modelMatrix, transform.location, transform.rotation, transform.scale);

        node->location[0] = transform.location[0];
        node->location[1] = transform.location[1];
        node->location[2] = transform.location[2];

        node->rotation[0] = transform.rotation[0];
        node->rotation[1] = transform.rotation[1];
        node->rotation[2] = transform.rotation[2];

        node->scale[0] = transform.scale[0];
        node->scale[1] = transform.scale[1];
        node->scale[2] = transform.scale[2];
    }

    // Track Undo/Redo drag states across frames
    bool isUsingNow = ImGuizmo::IsUsing();

    if (!s_WasUsingLastFrame && isUsingNow) {
        s_PreDragTransform = transform;
        s_DragNodeName = selectedNode;
        s_DragNodeId = node->id;
    }

    if (s_WasUsingLastFrame && !isUsingNow) {
        if (!s_DragNodeName.empty()) {
            auto cmd = std::make_shared<TransformChangeCommand>(s_DragNodeName, s_DragNodeId, s_PreDragTransform, transform);
            CommandStack::Get().PushAndExecute(cmd);
        }
        s_DragNodeName.clear();
        s_DragNodeId = 0;
    }

    s_WasUsingLastFrame = isUsingNow;
}

} // namespace EngineEditor::Panels
