#include "DetailsPanel.h"
#include "core/EditorState.h"
#include "core/ComponentRegistry.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include "widgets/PropertyRow.h"
#include "theme/Fonts.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "third_party/IconsFontAwesome6.h"

#include <unordered_map>
#include <functional>
#include <string>
#include <algorithm>
#include <cctype>
#include <imgui.h>
#include <imgui_internal.h>

namespace EngineEditor {

void RenderDetailsPanel(bool* pOpen) {
    if (!ImGui::Begin("Details", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const std::string& selectedNodeName = EditorState::Get().selectedNodeName;
    const std::string& selectedNodeType = EditorState::Get().selectedNodeType;

    const auto& pal = Theme::GetPalette();

    if (selectedNodeName.empty()) {
        ImGui::Dummy(ImVec2(0.0f, Theme::Metrics::sectionIndent));
        ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
        ImGui::TextColored(pal.textDisabled, "No selection.");
        ImGui::End();
        return;
    }

    SceneNode* activeNode = SceneGraph::Get().FindNodeMutable(selectedNodeName);
    uint64_t entityId = activeNode ? activeNode->id : 0;

    // 1. Header Block: Stacked Name & Type Layout with Type Icon + Right-aligned "Edit in C++"
    ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 headerStart = ImGui::GetCursorScreenPos();

    // Determine type icon color and shape
    ImVec2 iconBoxMin = ImVec2(headerStart.x, headerStart.y + 2.0f);
    ImVec2 iconBoxMax = ImVec2(iconBoxMin.x + 16.0f, iconBoxMin.y + 16.0f);

    if (selectedNodeType == "Folder" || selectedNodeName.find("Folder") != std::string::npos) {
        ImU32 folderCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.75f, 0.35f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 3.0f), ImVec2(iconBoxMax.x - 1.0f, iconBoxMax.y - 1.0f), folderCol, 2.0f);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 1.0f), ImVec2(iconBoxMin.x + 7.0f, iconBoxMin.y + 4.0f), folderCol, 1.0f);
    } else if (selectedNodeType == "Light" || selectedNodeName.find("Light") != std::string::npos) {
        ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.96f, 0.82f, 0.28f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.5f, lightCol);
    } else if (selectedNodeType == "Camera" || selectedNodeName.find("Camera") != std::string::npos) {
        ImU32 camCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.85f, 0.50f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 4.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), camCol, 2.0f);
    } else {
        ImU32 meshCol = ImGui::ColorConvertFloat4ToU32(pal.accent);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 2.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), meshCol, 2.0f);
    }

    float textX = Theme::Metrics::panelLeftMargin + 22.0f;

    // Right-aligned "Edit in C++" button aligned directly with top header row
    const char* editCppLabel = "Edit in C++";
    float editBtnWidth = ImGui::CalcTextSize(editCppLabel).x + 20.0f;
    float editBtnX = ImGui::GetWindowWidth() - editBtnWidth - Theme::Metrics::panelLeftMargin;

    if (editBtnX > textX + 100.0f) {
        ImGui::SetCursorScreenPos(ImVec2(editBtnX, headerStart.y + 4.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));

        if (ImGui::Button(editCppLabel)) {
            Logger::Get().Info("[Details] Edit in C++ clicked for " + selectedNodeName);
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

    // Stacked text layout on left side (Name + Type)
    ImGui::SetCursorScreenPos(ImVec2(headerStart.x + 22.0f, headerStart.y));
    ImGui::BeginGroup();

    // Line 1: Object Name (Bold/Semibold)
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.accent, "%s", selectedNodeName.c_str());
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PopFont();

    // Line 2: Type Label (Smaller, Muted Text directly below)
    std::string componentName = selectedNodeType.empty() ? "ActorComponent" : (selectedNodeType + "Component");
    ImGui::TextColored(pal.textDisabled, "%s", componentName.c_str());

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();

    // Collapsible Header Helper with Gear Options Menu
    auto RenderCollapsibleHeaderWithGear = [&](const char* label, const char* compId, std::function<void()> onReset, std::function<void()> onRemove) -> bool {
        static std::unordered_map<std::string, bool> s_States;
        std::string stateKey = std::string(label) + "_" + std::to_string(entityId);
        if (s_States.find(stateKey) == s_States.end()) s_States[stateKey] = true;
        bool& open = s_States[stateKey];

        ImGui::Spacing();

        ImVec2 hMin = ImGui::GetCursorScreenPos();
        float availW = ImGui::GetContentRegionAvail().x;
        float hHeight = 26.0f;
        ImVec2 hMax = ImVec2(hMin.x + availW, hMin.y + hHeight);

        ImGui::ItemSize(ImVec2(availW, hHeight));
        ImGui::ItemAdd(ImRect(hMin, hMax), ImGui::GetID(stateKey.c_str()));

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        if (clicked) {
            open = !open;
        }

        // Header background fill
        ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(hovered ? pal.bgElevated : pal.bgHeader);
        dl->AddRectFilled(hMin, hMax, bgCol, 2.0f);

        // Caret Triangle
        float caretX = hMin.x + Theme::Metrics::panelLeftMargin;
        float centerY = hMin.y + hHeight * 0.5f;
        ImU32 caretCol = ImGui::ColorConvertFloat4ToU32(pal.textSecondary);

        if (open) {
            ImVec2 p1(caretX, centerY - 3.0f);
            ImVec2 p2(caretX + 8.0f, centerY - 3.0f);
            ImVec2 p3(caretX + 4.0f, centerY + 3.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretCol);
        } else {
            ImVec2 p1(caretX + 2.0f, centerY - 4.0f);
            ImVec2 p2(caretX + 7.0f, centerY);
            ImVec2 p3(caretX + 2.0f, centerY + 4.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretCol);
        }

        // Title text
        ImVec2 textPos = ImVec2(caretX + 16.0f, hMin.y + (hHeight - ImGui::GetTextLineHeight()) * 0.5f);
        dl->AddText(textPos, ImGui::ColorConvertFloat4ToU32(pal.textPrimary), label);

        // Gear Option Menu on right side
        if (onReset || onRemove) {
            std::string gearBtnId = std::string("##Gear_") + label + "_" + std::to_string(entityId);
            std::string gearPopupId = std::string("GearMenu_") + label + "_" + std::to_string(entityId);

            float gearWidth = 22.0f;
            ImGui::SetCursorScreenPos(ImVec2(hMax.x - gearWidth - 4.0f, hMin.y + 2.0f));

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));

            if (ImGui::Button(gearBtnId.c_str(), ImVec2(20.0f, 22.0f))) {
                ImGui::OpenPopup(gearPopupId.c_str());
            }
            // Draw gear icon over button
            dl->AddText(ImVec2(hMax.x - gearWidth + 2.0f, textPos.y), ImGui::ColorConvertFloat4ToU32(pal.textDisabled), ICON_FA_GEAR);

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(2);

            if (ImGui::BeginPopup(gearPopupId.c_str())) {
                if (onReset && ImGui::MenuItem("Reset to Default")) {
                    onReset();
                }
                if (onRemove && ImGui::MenuItem("Remove Component")) {
                    onRemove();
                }
                ImGui::EndPopup();
            }
        }

        ImGui::Spacing();
        return open;
    };

    // 2. Transform Component ("Transition")
    TransformComponent* transformComp = entityId ? ComponentRegistry::Get().GetComponent<TransformComponent>(entityId) : nullptr;

    bool transitionOpen = RenderCollapsibleHeaderWithGear("Transition", "Transform", nullptr, nullptr);
    if (transitionOpen) {
        ImGui::Indent(Theme::Metrics::panelLeftMargin);
        ImGui::Spacing();

        auto& transform = EditorState::Get().activeTransform;

        bool locChanged = Widgets::RenderVector3PropertyRow("Location", transform.location, 0.0f);
        ImGui::Spacing();
        bool rotChanged = Widgets::RenderVector3PropertyRow("Rotation", transform.rotation, 0.0f);
        ImGui::Spacing();
        bool sclChanged = Widgets::RenderVector3PropertyRow("Scale", transform.scale, 1.0f, &transform.lockAspect);

        bool transformChanged = locChanged || rotChanged || sclChanged;

        // Sync back to SceneNode & ComponentRegistry
        if (activeNode && transformChanged) {
            activeNode->location[0] = transform.location[0];
            activeNode->location[1] = transform.location[1];
            activeNode->location[2] = transform.location[2];
            activeNode->rotation[0] = transform.rotation[0];
            activeNode->rotation[1] = transform.rotation[1];
            activeNode->rotation[2] = transform.rotation[2];
            activeNode->scale[0]    = transform.scale[0];
            activeNode->scale[1]    = transform.scale[1];
            activeNode->scale[2]    = transform.scale[2];

            if (transformComp) {
                transformComp->location[0] = transform.location[0];
                transformComp->location[1] = transform.location[1];
                transformComp->location[2] = transform.location[2];
                transformComp->rotation[0] = transform.rotation[0];
                transformComp->rotation[1] = transform.rotation[1];
                transformComp->rotation[2] = transform.rotation[2];
                transformComp->scale[0]    = transform.scale[0];
                transformComp->scale[1]    = transform.scale[1];
                transformComp->scale[2]    = transform.scale[2];
            }
        }

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::panelLeftMargin);
    }

    // 3. Dynamic Component Inspectors (Driven by ComponentRegistry)
    if (entityId != 0) {

        // --- MeshComponent Inspector ---
        MeshComponent* meshComp = ComponentRegistry::Get().GetComponent<MeshComponent>(entityId);
        if (meshComp) {
            bool meshOpen = RenderCollapsibleHeaderWithGear(
                "Mesh Component", "Mesh",
                [meshComp]() { *meshComp = MeshComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<MeshComponent>(entityId); }
            );

            if (meshOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                char meshPathBuf[256];
                strncpy_s(meshPathBuf, meshComp->meshPath.c_str(), sizeof(meshPathBuf));
                ImGui::TextUnformatted("Mesh Asset Path:");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##MeshPath", meshPathBuf, sizeof(meshPathBuf))) {
                    meshComp->meshPath = meshPathBuf;
                    if (activeNode) activeNode->meshPath = meshPathBuf;
                }

                ImGui::TextUnformatted("LOD Bias:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##LodBias", &meshComp->lodBias, -2.0f, 2.0f, "%.2f");

                ImGui::Checkbox("Cast Shadows", &meshComp->castShadows);
                ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
                ImGui::Checkbox("Receive Shadows", &meshComp->receiveShadows);
                ImGui::Checkbox("Show Bounding Box", &meshComp->showBoundingBox);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- MaterialComponent Inspector ---
        MaterialComponent* matComp = ComponentRegistry::Get().GetComponent<MaterialComponent>(entityId);
        if (matComp) {
            bool matOpen = RenderCollapsibleHeaderWithGear(
                "Material Component", "Material",
                [matComp]() { *matComp = MaterialComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<MaterialComponent>(entityId); }
            );

            if (matOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                char matPathBuf[256];
                strncpy_s(matPathBuf, matComp->materialPath.c_str(), sizeof(matPathBuf));
                ImGui::TextUnformatted("Material Path:");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##MatPath", matPathBuf, sizeof(matPathBuf))) {
                    matComp->materialPath = matPathBuf;
                    if (activeNode) activeNode->materialPath = matPathBuf;
                }

                ImGui::TextUnformatted("Base Color:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit4("##BaseColor", matComp->baseColor);

                ImGui::TextUnformatted("Roughness:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##Roughness", &matComp->roughness, 0.0f, 1.0f);

                ImGui::TextUnformatted("Metallic:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##Metallic", &matComp->metallic, 0.0f, 1.0f);

                ImGui::TextUnformatted("Specular:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##Specular", &matComp->specular, 0.0f, 1.0f);

                ImGui::TextUnformatted("Emissive Color:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit3("##EmissiveColor", matComp->emissiveColor);

                ImGui::TextUnformatted("Emissive Intensity:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##EmissiveIntensity", &matComp->emissiveIntensity, 0.1f, 0.0f, 100.0f, "%.1f");

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- LightComponent Inspector ---
        LightComponent* lightComp = ComponentRegistry::Get().GetComponent<LightComponent>(entityId);
        if (lightComp) {
            bool lightOpen = RenderCollapsibleHeaderWithGear(
                "Light Component", "Light",
                [lightComp]() { *lightComp = LightComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<LightComponent>(entityId); }
            );

            if (lightOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                const char* lightTypes[] = { "Directional Light", "Point Light", "Spot Light" };
                ImGui::TextUnformatted("Light Type:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::Combo("##LightType", &lightComp->lightType, lightTypes, 3);

                ImGui::TextUnformatted("Light Color:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit3("##LightColor", lightComp->color);

                ImGui::TextUnformatted("Intensity (Lux):");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##LightIntensity", &lightComp->intensity, 10.0f, 0.0f, 500000.0f, "%.0f Lux");

                ImGui::TextUnformatted("Range (m):");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##LightRange", &lightComp->range, 0.5f, 0.1f, 1000.0f, "%.1f m");

                if (lightComp->lightType == 2) { // Spot Light
                    ImGui::TextUnformatted("Inner Cone Angle:");
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::SliderFloat("##InnerCone", &lightComp->innerCone, 0.0f, 80.0f, "%.1f deg");

                    ImGui::TextUnformatted("Outer Cone Angle:");
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::SliderFloat("##OuterCone", &lightComp->outerCone, 0.0f, 89.0f, "%.1f deg");
                }

                ImGui::Checkbox("Cast Shadows", &lightComp->castShadows);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- CameraComponent Inspector ---
        CameraComponent* camComp = ComponentRegistry::Get().GetComponent<CameraComponent>(entityId);
        if (camComp) {
            bool camOpen = RenderCollapsibleHeaderWithGear(
                "Camera Component", "Camera",
                [camComp]() { *camComp = CameraComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<CameraComponent>(entityId); }
            );

            if (camOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                ImGui::TextUnformatted("Field of View (FOV):");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##CamFov", &camComp->fov, 10.0f, 170.0f, "%.1f deg");

                ImGui::TextUnformatted("Near Clip Plane:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##CamNear", &camComp->nearPlane, 0.01f, 0.001f, 10.0f, "%.3f m");

                ImGui::TextUnformatted("Far Clip Plane:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##CamFar", &camComp->farPlane, 10.0f, 1.0f, 100000.0f, "%.0f m");

                const char* projModes[] = { "Perspective", "Orthographic" };
                ImGui::TextUnformatted("Projection Mode:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::Combo("##CamProj", &camComp->projectionMode, projModes, 2);

                ImGui::TextUnformatted("Camera Priority:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragInt("##CamPriority", &camComp->priority, 1, 0, 100);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- RigidBodyComponent Inspector ---
        RigidBodyComponent* rbComp = ComponentRegistry::Get().GetComponent<RigidBodyComponent>(entityId);
        if (rbComp) {
            bool rbOpen = RenderCollapsibleHeaderWithGear(
                "RigidBody Component", "RigidBody",
                [rbComp]() { *rbComp = RigidBodyComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<RigidBodyComponent>(entityId); }
            );

            if (rbOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                ImGui::TextUnformatted("Mass (kg):");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##RBMass", &rbComp->mass, 0.5f, 0.0f, 10000.0f, "%.1f kg");

                ImGui::TextUnformatted("Linear Damping:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##RBLinearDamping", &rbComp->linearDamping, 0.01f, 0.0f, 10.0f, "%.2f");

                ImGui::TextUnformatted("Angular Damping:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##RBAngularDamping", &rbComp->angularDamping, 0.01f, 0.0f, 10.0f, "%.2f");

                ImGui::Checkbox("Is Kinematic", &rbComp->isKinematic);
                ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
                ImGui::Checkbox("Use Gravity", &rbComp->useGravity);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- ColliderComponent Inspector ---
        ColliderComponent* colComp = ComponentRegistry::Get().GetComponent<ColliderComponent>(entityId);
        if (colComp) {
            bool colOpen = RenderCollapsibleHeaderWithGear(
                "Collider Component", "Collider",
                [colComp]() { *colComp = ColliderComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<ColliderComponent>(entityId); }
            );

            if (colOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                const char* shapeTypes[] = { "Box", "Sphere", "Capsule", "Mesh" };
                ImGui::TextUnformatted("Collider Shape:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::Combo("##ColShape", &colComp->shapeType, shapeTypes, 4);

                if (colComp->shapeType == 0) { // Box
                    Widgets::RenderVector3PropertyRow("Box Size", colComp->size, 1.0f);
                } else if (colComp->shapeType == 1) { // Sphere
                    ImGui::TextUnformatted("Radius:");
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::DragFloat("##ColRadius", &colComp->radius, 0.1f, 0.01f, 100.0f, "%.2f m");
                } else if (colComp->shapeType == 2) { // Capsule
                    ImGui::TextUnformatted("Radius:");
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::DragFloat("##ColCapRadius", &colComp->radius, 0.1f, 0.01f, 100.0f, "%.2f m");
                    ImGui::TextUnformatted("Height:");
                    ImGui::SetNextItemWidth(-1.0f);
                    ImGui::DragFloat("##ColCapHeight", &colComp->height, 0.1f, 0.01f, 100.0f, "%.2f m");
                }

                ImGui::Checkbox("Is Trigger Volume", &colComp->isTrigger);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- AudioSourceComponent Inspector ---
        AudioSourceComponent* audioComp = ComponentRegistry::Get().GetComponent<AudioSourceComponent>(entityId);
        if (audioComp) {
            bool audioOpen = RenderCollapsibleHeaderWithGear(
                "Audio Source Component", "AudioSource",
                [audioComp]() { *audioComp = AudioSourceComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<AudioSourceComponent>(entityId); }
            );

            if (audioOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                char clipBuf[256];
                strncpy_s(clipBuf, audioComp->clipPath.c_str(), sizeof(clipBuf));
                ImGui::TextUnformatted("Audio Clip Path:");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##AudioClip", clipBuf, sizeof(clipBuf))) {
                    audioComp->clipPath = clipBuf;
                }

                ImGui::TextUnformatted("Volume:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##AudioVolume", &audioComp->volume, 0.0f, 1.0f, "%.2f");

                ImGui::TextUnformatted("Pitch:");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##AudioPitch", &audioComp->pitch, 0.1f, 3.0f, "%.2fx");

                ImGui::TextUnformatted("Spatial Blend (2D <-> 3D):");
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::SliderFloat("##AudioSpatial", &audioComp->spatialBlend, 0.0f, 1.0f, "%.2f");

                ImGui::Checkbox("Looping", &audioComp->loop);
                ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
                ImGui::Checkbox("Play On Awake", &audioComp->playOnAwake);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // --- ScriptComponent Inspector ---
        ScriptComponent* scriptComp = ComponentRegistry::Get().GetComponent<ScriptComponent>(entityId);
        if (scriptComp) {
            bool scriptOpen = RenderCollapsibleHeaderWithGear(
                "Script Component", "Script",
                [scriptComp]() { *scriptComp = ScriptComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<ScriptComponent>(entityId); }
            );

            if (scriptOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                char scriptBuf[256];
                strncpy_s(scriptBuf, scriptComp->scriptPath.c_str(), sizeof(scriptBuf));
                ImGui::TextUnformatted("Script File Path:");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##ScriptPath", scriptBuf, sizeof(scriptBuf))) {
                    scriptComp->scriptPath = scriptBuf;
                }

                char classBuf[128];
                strncpy_s(classBuf, scriptComp->className.c_str(), sizeof(classBuf));
                ImGui::TextUnformatted("Class Name:");
                ImGui::SetNextItemWidth(-1.0f);
                if (ImGui::InputText("##ClassName", classBuf, sizeof(classBuf))) {
                    scriptComp->className = classBuf;
                }

                ImGui::Checkbox("Script Enabled", &scriptComp->enabled);

                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // Retain SkyAtmosphere Section for SkyAtmosphere Component / Nodes
        SkyAtmosphereComponent* skyComp = ComponentRegistry::Get().GetComponent<SkyAtmosphereComponent>(entityId);
        if (skyComp || selectedNodeName == "SkyAtmosphere" || selectedNodeType == "SkyAtmosphere") {
            bool skyOpen = RenderCollapsibleHeaderWithGear(
                "Sky Atmosphere", "Sky",
                [skyComp]() { if (skyComp) *skyComp = SkyAtmosphereComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<SkyAtmosphereComponent>(entityId); }
            );

            if (skyOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                if (!skyComp) {
                    skyComp = ComponentRegistry::Get().AddComponent<SkyAtmosphereComponent>(entityId);
                }

                ImGui::Columns(2, "##SkyProps", false);
                ImGui::SetColumnWidth(0, Theme::Metrics::labelColumnWidth * 2.0f);

                ImGui::TextUnformatted("Enable Sky Atmosphere");
                ImGui::NextColumn();
                ImGui::Checkbox("##SkyEnable", &skyComp->enabled);
                ImGui::NextColumn();

                ImGui::TextUnformatted("Sky Intensity");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##SkyIntensity", &skyComp->skyIntensity, 0.05f, 0.00f, 10.00f, "%.2f");
                ImGui::NextColumn();

                ImGui::TextUnformatted("Zenith Color");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit3("##ZenithColor", skyComp->zenithColor);
                ImGui::NextColumn();

                ImGui::TextUnformatted("Horizon Color");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit3("##HorizonColor", skyComp->horizonColor);
                ImGui::NextColumn();

                ImGui::TextUnformatted("Rayleigh Scattering Scale");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##RayleighScattering", &skyComp->rayleighScattering, 0.001f, 0.000f, 1.000f, "%.4f");
                ImGui::NextColumn();

                ImGui::TextUnformatted("Atmosphere Height");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##AtmosphereHeight", &skyComp->atmosphereHeightKm, 0.5f, 1.0f, 100.0f, "%.1f km");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }

        // Retain Volumetric Fog Section for VolumetricFog Component / Nodes
        VolumetricFogComponent* fogComp = ComponentRegistry::Get().GetComponent<VolumetricFogComponent>(entityId);
        if (fogComp || selectedNodeName == "VolumetricFog" || selectedNodeType == "VolumetricFog") {
            bool fogOpen = RenderCollapsibleHeaderWithGear(
                "Volumetric Fog", "Fog",
                [fogComp]() { if (fogComp) *fogComp = VolumetricFogComponent(); },
                [entityId]() { ComponentRegistry::Get().RemoveComponent<VolumetricFogComponent>(entityId); }
            );

            if (fogOpen) {
                ImGui::Indent(Theme::Metrics::panelLeftMargin);
                ImGui::Spacing();

                if (!fogComp) {
                    fogComp = ComponentRegistry::Get().AddComponent<VolumetricFogComponent>(entityId);
                }

                ImGui::Columns(2, "##FogProps", false);
                ImGui::SetColumnWidth(0, Theme::Metrics::labelColumnWidth * 2.0f);

                ImGui::TextUnformatted("Enable Volumetric Fog");
                ImGui::NextColumn();
                ImGui::Checkbox("##FogEnable", &fogComp->enabled);
                ImGui::NextColumn();

                ImGui::TextUnformatted("Fog Density");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##FogDensity", &fogComp->density, 0.001f, 0.000f, 0.500f, "%.4f");
                ImGui::NextColumn();

                ImGui::TextUnformatted("Fog Color");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::ColorEdit3("##FogColor", fogComp->color);
                ImGui::NextColumn();

                ImGui::TextUnformatted("Start Distance");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##FogStart", &fogComp->startDistance, 1.0f, 0.0f, 1000.0f, "%.1f m");
                ImGui::NextColumn();

                ImGui::TextUnformatted("End Distance");
                ImGui::NextColumn();
                ImGui::SetNextItemWidth(-1.0f);
                ImGui::DragFloat("##FogEnd", &fogComp->endDistance, 5.0f, 10.0f, 5000.0f, "%.1f m");

                ImGui::Columns(1);
                ImGui::Spacing();
                ImGui::Unindent(Theme::Metrics::panelLeftMargin);
            }
        }
    }

    // 4. "+ Add Component" Button & Popup Menu
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
    float availWidth = ImGui::GetContentRegionAvail().x;

    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));

    if (ImGui::Button("+ Add Component", ImVec2(availWidth, 30.0f))) {
        ImGui::OpenPopup("AddComponentPopup");
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopup("AddComponentPopup")) {
        ImGui::TextColored(pal.textDisabled, "SELECT COMPONENT TO ADD");
        ImGui::Separator();

        static char searchFilter[64] = "";
        ImGui::SetNextItemWidth(220.0f);
        ImGui::InputTextWithHint("##CompSearch", "Search components...", searchFilter, sizeof(searchFilter));
        ImGui::Separator();

        std::string filter = searchFilter;
        std::transform(filter.begin(), filter.end(), filter.begin(), [](unsigned char c) { return (char)std::tolower(c); });

        auto MatchesFilter = [&](const char* name) -> bool {
            if (filter.empty()) return true;
            std::string n = name;
            std::transform(n.begin(), n.end(), n.begin(), [](unsigned char c) { return (char)std::tolower(c); });
            return n.find(filter) != std::string::npos;
        };

        if (ImGui::BeginMenu("Rendering")) {
            if (MatchesFilter("Static Mesh") && ImGui::MenuItem(ICON_FA_CUBE " Static Mesh")) {
                ComponentRegistry::Get().AddComponent<MeshComponent>(entityId);
            }
            if (MatchesFilter("Material") && ImGui::MenuItem(ICON_FA_PALETTE " Material")) {
                ComponentRegistry::Get().AddComponent<MaterialComponent>(entityId);
            }
            if (MatchesFilter("Camera") && ImGui::MenuItem(ICON_FA_CAMERA " Camera")) {
                ComponentRegistry::Get().AddComponent<CameraComponent>(entityId);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Lighting")) {
            if (MatchesFilter("Point Light") && ImGui::MenuItem(ICON_FA_LIGHTBULB " Point Light")) {
                LightComponent lc; lc.lightType = 1; lc.intensity = 2500.0f; lc.range = 15.0f; lc.color[0] = 1.0f; lc.color[1] = 0.90f; lc.color[2] = 0.70f;
                ComponentRegistry::Get().AddComponent<LightComponent>(entityId, lc);
            }
            if (MatchesFilter("Spot Light") && ImGui::MenuItem(ICON_FA_LIGHTBULB " Spot Light")) {
                LightComponent lc; lc.lightType = 2; lc.intensity = 5000.0f; lc.range = 25.0f; lc.color[0] = 1.0f; lc.color[1] = 0.90f; lc.color[2] = 0.70f;
                ComponentRegistry::Get().AddComponent<LightComponent>(entityId, lc);
            }
            if (MatchesFilter("Directional Light") && ImGui::MenuItem(ICON_FA_SUN " Directional Light")) {
                LightComponent lc; lc.lightType = 0; lc.intensity = 100000.0f; lc.color[0] = 1.0f; lc.color[1] = 0.95f; lc.color[2] = 0.85f;
                ComponentRegistry::Get().AddComponent<LightComponent>(entityId, lc);
            }
            if (MatchesFilter("Sky Atmosphere") && ImGui::MenuItem(ICON_FA_SUN " Sky Atmosphere")) {
                ComponentRegistry::Get().AddComponent<SkyAtmosphereComponent>(entityId);
            }
            if (MatchesFilter("Volumetric Fog") && ImGui::MenuItem(ICON_FA_SUN " Volumetric Fog")) {
                ComponentRegistry::Get().AddComponent<VolumetricFogComponent>(entityId);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Physics")) {
            if (MatchesFilter("RigidBody 3D") && ImGui::MenuItem(ICON_FA_CUBE " RigidBody 3D")) {
                ComponentRegistry::Get().AddComponent<RigidBodyComponent>(entityId);
            }
            if (MatchesFilter("Box Collider") && ImGui::MenuItem(ICON_FA_CUBE " Box Collider")) {
                ColliderComponent cc; cc.shapeType = 0;
                ComponentRegistry::Get().AddComponent<ColliderComponent>(entityId, cc);
            }
            if (MatchesFilter("Sphere Collider") && ImGui::MenuItem(ICON_FA_CUBE " Sphere Collider")) {
                ColliderComponent cc; cc.shapeType = 1;
                ComponentRegistry::Get().AddComponent<ColliderComponent>(entityId, cc);
            }
            if (MatchesFilter("Capsule Collider") && ImGui::MenuItem(ICON_FA_CUBE " Capsule Collider")) {
                ColliderComponent cc; cc.shapeType = 2;
                ComponentRegistry::Get().AddComponent<ColliderComponent>(entityId, cc);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Audio")) {
            if (MatchesFilter("Audio Source") && ImGui::MenuItem(ICON_FA_VOLUME_HIGH " Audio Source")) {
                ComponentRegistry::Get().AddComponent<AudioSourceComponent>(entityId);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Scripting")) {
            if (MatchesFilter("Script") && ImGui::MenuItem(ICON_FA_CODE " Script")) {
                ComponentRegistry::Get().AddComponent<ScriptComponent>(entityId);
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace EngineEditor
