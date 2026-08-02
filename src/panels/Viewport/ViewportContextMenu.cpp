#include "ViewportContextMenu.h"
#include "ViewportSelection.h"
#include "ViewportMeasurement.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "core/CommandStack.h"
#include "engine/core/Logger.h"

namespace EngineEditor::Panels {

ViewportContextMenu& ViewportContextMenu::Get() {
    static ViewportContextMenu instance;
    return instance;
}

void ViewportContextMenu::OpenMenu(const std::string& targetNodeName) {
    m_TargetNode = targetNodeName;
    m_ShouldOpen = true;
}

void ViewportContextMenu::Render() {
    if (m_ShouldOpen) {
        ImGui::OpenPopup("Viewport3DContextMenu");
        m_ShouldOpen = false;
    }

    if (ImGui::BeginPopup("Viewport3DContextMenu")) {
        std::string selectedName = EditorState::Get().selectedNodeName;
        bool hasSelection = !selectedName.empty();

        if (hasSelection) {
            ImGui::TextDisabled("Object: %s", selectedName.c_str());
            ImGui::Separator();

            if (ImGui::MenuItem("Focus Camera", "F")) {
                std::vector<AABB> bounds;
                AABB b;
                bounds.push_back(b);
                EditorState::Get().camera.FrameSelection(bounds);
            }

            if (ImGui::MenuItem("Isolate Selection", "Alt+H")) {
                EditorState::Get().isIsolationMode = !EditorState::Get().isIsolationMode;
            }

            ImGui::Separator();
            if (ImGui::BeginMenu("Transform")) {
                if (ImGui::MenuItem("Snap to Ground", "End")) {
                    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedName);
                    if (node) {
                        TransformData oldT = EditorState::Get().activeTransform;
                        node->location[1] = 0.0f;
                        TransformData newT = oldT;
                        newT.location[1] = 0.0f;
                        auto cmd = std::make_shared<TransformChangeCommand>(selectedName, node->id, oldT, newT);
                        CommandStack::Get().PushAndExecute(cmd);
                    }
                }
                if (ImGui::MenuItem("Reset Translation")) {
                    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedName);
                    if (node) {
                        TransformData oldT = EditorState::Get().activeTransform;
                        node->location[0] = node->location[1] = node->location[2] = 0.0f;
                        TransformData newT = oldT;
                        newT.location[0] = newT.location[1] = newT.location[2] = 0.0f;
                        auto cmd = std::make_shared<TransformChangeCommand>(selectedName, node->id, oldT, newT);
                        CommandStack::Get().PushAndExecute(cmd);
                    }
                }
                if (ImGui::MenuItem("Reset Rotation")) {
                    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedName);
                    if (node) {
                        TransformData oldT = EditorState::Get().activeTransform;
                        node->rotation[0] = node->rotation[1] = node->rotation[2] = 0.0f;
                        TransformData newT = oldT;
                        newT.rotation[0] = newT.rotation[1] = newT.rotation[2] = 0.0f;
                        auto cmd = std::make_shared<TransformChangeCommand>(selectedName, node->id, oldT, newT);
                        CommandStack::Get().PushAndExecute(cmd);
                    }
                }
                if (ImGui::MenuItem("Reset Scale")) {
                    SceneNode* node = SceneGraph::Get().FindNodeMutable(selectedName);
                    if (node) {
                        TransformData oldT = EditorState::Get().activeTransform;
                        node->scale[0] = node->scale[1] = node->scale[2] = 1.0f;
                        TransformData newT = oldT;
                        newT.scale[0] = newT.scale[1] = newT.scale[2] = 1.0f;
                        auto cmd = std::make_shared<TransformChangeCommand>(selectedName, node->id, oldT, newT);
                        CommandStack::Get().PushAndExecute(cmd);
                    }
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Select")) {
                if (ImGui::MenuItem("Select Parent")) ViewportSelection::Get().SelectParent();
                if (ImGui::MenuItem("Select Children")) ViewportSelection::Get().SelectChildren();
                if (ImGui::MenuItem("Select All", "Ctrl+A")) ViewportSelection::Get().SelectAll();
                if (ImGui::MenuItem("Invert Selection", "Ctrl+I")) ViewportSelection::Get().InvertSelection();
                ImGui::EndMenu();
            }

            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                SceneGraph::Get().DuplicateNode(selectedName);
            }
            if (ImGui::MenuItem("Delete", "Del")) {
                SceneGraph::Get().RemoveNode(selectedName);
                EditorState::Get().ClearSelection();
            }
        } else {
            ImGui::TextDisabled("Viewport Options");
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) ViewportSelection::Get().SelectAll();
            if (ImGui::MenuItem("Measure Tool", "Shift+M")) ViewportMeasurement::Get().Activate();
            if (ImGui::MenuItem("Reset Camera", "Home")) EditorState::Get().camera.ResetToDefault();
        }

        ImGui::EndPopup();
    }
}

} // namespace EngineEditor::Panels
