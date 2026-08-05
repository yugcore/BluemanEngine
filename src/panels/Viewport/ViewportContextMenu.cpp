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
            if (ImGui::BeginMenu("Create")) {
                if (ImGui::BeginMenu("3D Object")) {
                    if (ImGui::MenuItem("Cube")) {
                        SceneNode cubeNode;
                        cubeNode.name = "Cube_" + std::to_string(rand() % 1000);
                        cubeNode.type = SceneNodeType::Actor;
                        cubeNode.location[0] = 0.0f; cubeNode.location[1] = 1.0f; cubeNode.location[2] = 0.0f;
                        cubeNode.meshPath = "Engine/DefaultCube";
                        cubeNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(cubeNode);
                        EditorState::Get().SetSelection(cubeNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Sphere")) {
                        SceneNode sphereNode;
                        sphereNode.name = "Sphere_" + std::to_string(rand() % 1000);
                        sphereNode.type = SceneNodeType::Actor;
                        sphereNode.location[0] = 0.0f; sphereNode.location[1] = 1.0f; sphereNode.location[2] = 0.0f;
                        sphereNode.meshPath = "Engine/DefaultSphere";
                        sphereNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(sphereNode);
                        EditorState::Get().SetSelection(sphereNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Cylinder")) {
                        SceneNode cylNode;
                        cylNode.name = "Cylinder_" + std::to_string(rand() % 1000);
                        cylNode.type = SceneNodeType::Actor;
                        cylNode.location[0] = 0.0f; cylNode.location[1] = 1.0f; cylNode.location[2] = 0.0f;
                        cylNode.meshPath = "Engine/DefaultCylinder";
                        cylNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(cylNode);
                        EditorState::Get().SetSelection(cylNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Plane")) {
                        SceneNode planeNode;
                        planeNode.name = "Plane_" + std::to_string(rand() % 1000);
                        planeNode.type = SceneNodeType::Actor;
                        planeNode.location[0] = 0.0f; planeNode.location[1] = 0.05f; planeNode.location[2] = 0.0f;
                        planeNode.meshPath = "Engine/DefaultPlane";
                        planeNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(planeNode);
                        EditorState::Get().SetSelection(planeNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Cone")) {
                        SceneNode coneNode;
                        coneNode.name = "Cone_" + std::to_string(rand() % 1000);
                        coneNode.type = SceneNodeType::Actor;
                        coneNode.location[0] = 0.0f; coneNode.location[1] = 1.0f; coneNode.location[2] = 0.0f;
                        coneNode.meshPath = "Engine/DefaultCone";
                        coneNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(coneNode);
                        EditorState::Get().SetSelection(coneNode.name, "Actor");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Light")) {
                    if (ImGui::MenuItem("Directional Sun Light")) {
                        SceneNode sunNode;
                        sunNode.name = "DirectionalSunLight_" + std::to_string(rand() % 1000);
                        sunNode.type = SceneNodeType::Light;
                        sunNode.location[0] = 0.0f; sunNode.location[1] = 10.0f; sunNode.location[2] = 0.0f;
                        sunNode.rotation[0] = 53.0f; sunNode.rotation[1] = -59.0f; sunNode.rotation[2] = 0.0f;
                        SceneGraph::Get().AddNode(sunNode);
                        EditorState::Get().SetSelection(sunNode.name, "Light");
                    }
                    if (ImGui::MenuItem("Point Light")) {
                        SceneNode ptNode;
                        ptNode.name = "PointLight_" + std::to_string(rand() % 1000);
                        ptNode.type = SceneNodeType::Light;
                        ptNode.location[0] = 0.0f; ptNode.location[1] = 3.0f; ptNode.location[2] = 0.0f;
                        SceneGraph::Get().AddNode(ptNode);
                        EditorState::Get().SetSelection(ptNode.name, "Light");
                    }
                    if (ImGui::MenuItem("Spot Light")) {
                        SceneNode spotNode;
                        spotNode.name = "SpotLight_" + std::to_string(rand() % 1000);
                        spotNode.type = SceneNodeType::Light;
                        spotNode.location[0] = 0.0f; spotNode.location[1] = 3.0f; spotNode.location[2] = 0.0f;
                        spotNode.rotation[0] = 45.0f; spotNode.rotation[1] = 0.0f; spotNode.rotation[2] = 0.0f;
                        SceneGraph::Get().AddNode(spotNode);
                        EditorState::Get().SetSelection(spotNode.name, "Light");
                    }
                    if (ImGui::MenuItem("SkyAtmosphere")) {
                        SceneNode skyNode;
                        skyNode.name = "SkyAtmosphere_" + std::to_string(rand() % 1000);
                        skyNode.type = SceneNodeType::SkyAtmosphere;
                        SceneGraph::Get().AddNode(skyNode);
                        EditorState::Get().SetSelection(skyNode.name, "SkyAtmosphere");
                    }
                    if (ImGui::MenuItem("Volumetric Fog")) {
                        SceneNode fogNode;
                        fogNode.name = "VolumetricFog_" + std::to_string(rand() % 1000);
                        fogNode.type = SceneNodeType::VolumetricFog;
                        SceneGraph::Get().AddNode(fogNode);
                        EditorState::Get().SetSelection(fogNode.name, "VolumetricFog");
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) ViewportSelection::Get().SelectAll();
            if (ImGui::MenuItem("Measure Tool", "Shift+M")) ViewportMeasurement::Get().Activate();
            if (ImGui::MenuItem("Reset Camera", "Home")) EditorState::Get().camera.ResetToDefault();
        }

        ImGui::EndPopup();
    }
}

} // namespace EngineEditor::Panels
