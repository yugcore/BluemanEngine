#include "Gizmos.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/CommandStack.h"
#include "third_party/ImGuizmo/ImGuizmo.h"

namespace EngineEditor::Panels {

namespace {

// Encapsulated drag state instead of loose file-local statics, so this can be
// made per-panel (keyed by an ImGuizmo ID) the moment a second viewport needs
// gizmo support (Mesh Studio, a picture-in-picture cam preview, etc).
struct GizmoDragState {
    bool wasUsingLastFrame = false;
    bool hasIdleSnapshot = false;
    TransformData idleTransform = {};   // last known-good transform while NOT dragging
    TransformData preDragTransform = {};
    std::string dragNodeName;
    uint64_t dragNodeId = 0;

    void Reset() {
        wasUsingLastFrame = false;
        hasIdleSnapshot = false;
        dragNodeName.clear();
        dragNodeId = 0;
    }
};

GizmoDragState s_State;

// Finalizes an in-flight drag (if any) by pushing the undo command, using
// whatever the transform was at the time of the call. Used both for the
// normal "mouse released" path and for abnormal termination (selection
// changed mid-drag, node deleted mid-drag, panel closed mid-drag, etc).
void FinalizeOrCancelDrag(const TransformData& currentTransform) {
    if (s_State.dragNodeName.empty()) {
        return;
    }
    // Only push a command if something actually changed; avoids polluting
    // the undo stack with no-op entries when a drag starts and immediately
    // gets interrupted with zero delta.
    bool changed = false;
    for (int i = 0; i < 3 && !changed; ++i) {
        if (s_State.preDragTransform.location[i] != currentTransform.location[i] ||
            s_State.preDragTransform.rotation[i] != currentTransform.rotation[i] ||
            s_State.preDragTransform.scale[i]    != currentTransform.scale[i]) {
            changed = true;
        }
    }
    if (changed) {
        auto cmd = std::make_shared<TransformChangeCommand>(
            s_State.dragNodeName, s_State.dragNodeId, s_State.preDragTransform, currentTransform);
        CommandStack::Get().PushAndExecute(cmd);
    }
    s_State.dragNodeName.clear();
    s_State.dragNodeId = 0;
}

bool IsCtrlHeld() {
#ifdef IMGUI_HAS_KEYMOD_HELPERS
    return ImGui::IsKeyDown(ImGuiMod_Ctrl);
#else
    return ImGui::GetIO().KeyCtrl;
#endif
}

} // anonymous namespace

void RenderViewportGizmos(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail) {
    std::string selectedNode = EditorState::Get().selectedNodeName;

    if (selectedNode.empty() || viewportAvail.x <= 0.0f || viewportAvail.y <= 0.0f) {
        // If selection was cleared (or viewport collapsed) mid-drag, don't just
        // drop the in-progress edit silently — finalize it with whatever the
        // last valid transform was.
        FinalizeOrCancelDrag(EditorState::Get().activeTransform);
        s_State.Reset();
        return;
    }

    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedNode);
    if (!node) {
        FinalizeOrCancelDrag(EditorState::Get().activeTransform);
        s_State.Reset();
        return;
    }

    // Selection changed to a different node mid-drag: finalize the old drag
    // before starting fresh tracking on the new node, rather than losing it.
    if (!s_State.dragNodeName.empty() && s_State.dragNodeName != selectedNode) {
        FinalizeOrCancelDrag(EditorState::Get().activeTransform);
        s_State.Reset();
    }

    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(drawList);
    ImGuizmo::SetRect(cursorPos.x, cursorPos.y, viewportAvail.x, viewportAvail.y);

    auto& transform = EditorState::Get().activeTransform;

    // ImGuizmo::IsUsing() here reflects the *previous* frame's result.
    const bool wasUsingBeforeManipulate = ImGuizmo::IsUsing();

    if (!wasUsingBeforeManipulate) {
        transform.location[0] = node->location[0];
        transform.location[1] = node->location[1];
        transform.location[2] = node->location[2];

        transform.rotation[0] = node->rotation[0];
        transform.rotation[1] = node->rotation[1];
        transform.rotation[2] = node->rotation[2];

        transform.scale[0] = node->scale[0];
        transform.scale[1] = node->scale[1];
        transform.scale[2] = node->scale[2];

        // Snapshot the idle (authoritative, un-dragged) transform every
        // frame we're not dragging. This is what a new drag's "before" state
        // is taken from — NOT the post-Manipulate value, which may already
        // include this frame's delta.
        s_State.idleTransform = transform;
        s_State.hasIdleSnapshot = true;
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

    // Handle Multi-Selection Group Center
    const auto& selectedNames = EditorState::Get().selectedNodeNames;
    if (selectedNames.size() > 1) {
        float avgLoc[3] = { 0.0f, 0.0f, 0.0f };
        int validCount = 0;
        for (const auto& name : selectedNames) {
            SceneNode* sNode = SceneGraph::Get().FindNodeMutable(name);
            if (sNode) {
                avgLoc[0] += sNode->location[0];
                avgLoc[1] += sNode->location[1];
                avgLoc[2] += sNode->location[2];
                validCount++;
            }
        }
        if (validCount > 0) {
            transform.location[0] = avgLoc[0] / (float)validCount;
            transform.location[1] = avgLoc[1] / (float)validCount;
            transform.location[2] = avgLoc[2] / (float)validCount;
        }
    }

    // Support Pivot Editing Mode
    if (EditorState::Get().isPivotEditingMode) {
        transform.location[0] += EditorState::Get().customPivotOffset[0];
        transform.location[1] += EditorState::Get().customPivotOffset[1];
        transform.location[2] += EditorState::Get().customPivotOffset[2];
    }

    // Force LOCAL mode for scale operations
    ImGuizmo::MODE mode = (op == ImGuizmo::SCALE)
        ? ImGuizmo::LOCAL
        : ((EditorState::Get().activeTransformSpace == TransformSpace::World) ? ImGuizmo::WORLD : ImGuizmo::LOCAL);

    // Dynamic snap settings. Ctrl temporarily inverts whatever the panel
    // checkbox says (hold-to-toggle), hence the explicit != rather than a
    // bitwise XOR on bools — same result, clearer intent at the call site.
    const auto& snap = EditorState::Get().snapSettings;
    const bool ctrlHeld = IsCtrlHeld();
    float snapValues[3] = { 0.0f, 0.0f, 0.0f };
    float* pSnap = nullptr;

    if (op == ImGuizmo::TRANSLATE && (snap.enableTranslate != ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.translateSnap;
        pSnap = snapValues;
    } else if (op == ImGuizmo::ROTATE && (snap.enableRotate != ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.rotateSnap;
        pSnap = snapValues;
    } else if (op == ImGuizmo::SCALE && (snap.enableScale != ctrlHeld)) {
        snapValues[0] = snapValues[1] = snapValues[2] = snap.scaleSnap;
        pSnap = snapValues;
    }

    // Gate gizmo interaction behind camera state (disable while Fly/Orbit/Pan).
    const bool isCameraIdle = (camera.GetMode() == CameraMode::Idle);
    ImGuizmo::Enable(isCameraIdle);

    ImGuizmo::Manipulate(viewMatrix, projMatrix, op, mode, modelMatrix, nullptr, pSnap);

    const bool isUsingNow = ImGuizmo::IsUsing();

    if (isUsingNow) {
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

    // Drag start: base the "before" snapshot on the last idle transform we
    // captured (above, before Manipulate ran this frame), not on `transform`
    // itself, which may already contain this frame's delta.
    if (!s_State.wasUsingLastFrame && isUsingNow) {
        s_State.preDragTransform = s_State.hasIdleSnapshot ? s_State.idleTransform : transform;
        s_State.dragNodeName = selectedNode;
        s_State.dragNodeId = node->id;
    }

    // Drag end: push the undo command.
    if (s_State.wasUsingLastFrame && !isUsingNow) {
        FinalizeOrCancelDrag(transform);
    }

    s_State.wasUsingLastFrame = isUsingNow;
}

} // namespace EngineEditor::Panels