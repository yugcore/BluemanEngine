#include "ViewportPanel.h"
#include "Overlay.h"
#include "Gizmos.h"
#include "ViewportMath.h"
#include "ViewportPicker.h"
#include "ViewportSelection.h"
#include "ViewportMeasurement.h"
#include "ViewportContextMenu.h"
#include "ViewportPreferences.h"
#include "ViewportDragDrop.h"
#include "render/ViewportRenderer.h"
#include "render/ZeGFXAdapter.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/CommandStack.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <imgui_internal.h>
#include "third_party/ImGuizmo/ImGuizmo.h"
#include <cstdio>
#include <filesystem>

namespace EngineEditor {

static int s_PerspectiveIdx = 0;
static int s_QualityIdx = 0;
static int s_ShowFlags = 7;

static void RenderStatusPill(const char* label, const ImVec4& color) {
    const auto& pal = Theme::GetPalette();
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

    float deltaTime = io.DeltaTime;

    // Update Camera
    auto& camera = EditorState::Get().camera;
    camera.Update(deltaTime, inputState);

    // Handle Viewport Hotkeys when in Idle Camera Mode & Viewport is Hovered/Focused
    if (camera.GetMode() == CameraMode::Idle && (isHovered || isFocused) && !io.WantTextInput) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) EditorState::Get().gizmoOp = GizmoOperation::Translate;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) EditorState::Get().gizmoOp = GizmoOperation::Rotate;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) EditorState::Get().gizmoOp = GizmoOperation::Scale;

        // Space key: Cycle World/Local space
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            EditorState::Get().activeTransformSpace = (EditorState::Get().activeTransformSpace == TransformSpace::World) 
                ? TransformSpace::Local : TransformSpace::World;
        }

        // F key: Frame selection
        if (ImGui::IsKeyPressed(ImGuiKey_F)) {
            std::vector<AABB> bounds;
            AABB b;
            b.minBounds = Vec3f(-1.0f, -1.0f, -1.0f);
            b.maxBounds = Vec3f(1.0f, 1.0f, 1.0f);
            bounds.push_back(b);
            camera.FrameSelection(bounds);
        }

        // Home key: Reset camera
        if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
            camera.ResetToDefault();
        }

        // Insert / D key: Pivot edit mode toggle
        if (ImGui::IsKeyPressed(ImGuiKey_Insert) || ImGui::IsKeyPressed(ImGuiKey_D)) {
            EditorState::Get().isPivotEditingMode = !EditorState::Get().isPivotEditingMode;
        }

        // Alt + H: Isolate Selection toggle
        if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_H)) {
            EditorState::Get().isIsolationMode = !EditorState::Get().isIsolationMode;
        }

        // Ctrl + A: Select All
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_A)) {
            Panels::ViewportSelection::Get().SelectAll();
        }

        // Ctrl + I: Invert Selection
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_I)) {
            Panels::ViewportSelection::Get().InvertSelection();
        }

        // Ctrl + Z: Undo, Ctrl + Y: Redo
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
            CommandStack::Get().Undo();
        }
        if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
            CommandStack::Get().Redo();
        }

        // Camera Bookmarks: Ctrl+Shift+0..9 to Save, Ctrl+0..9 to Load
        for (int bIdx = 0; bIdx < 10; ++bIdx) {
            ImGuiKey numKey = (ImGuiKey)(ImGuiKey_0 + bIdx);
            if (ImGui::IsKeyPressed(numKey)) {
                if (io.KeyCtrl && io.KeyShift) {
                    camera.SaveBookmark(bIdx);
                    Logger::Get().Info("[Viewport] Saved camera bookmark slot " + std::to_string(bIdx));
                } else if (io.KeyCtrl) {
                    if (camera.LoadBookmark(bIdx)) {
                        Logger::Get().Info("[Viewport] Loaded camera bookmark slot " + std::to_string(bIdx));
                    }
                }
            }
        }
    }

    ViewportRenderer::Get().RenderScene(deltaTime);

    uint64_t textureID = ViewportRenderer::Get().GetTextureID();
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();

    if (textureID != 0) {
        ImGui::Image((ImTextureID)textureID, viewportAvail, ImVec2(0, 1), ImVec2(1, 0));
    } else {
        ImGui::Dummy(viewportAvail);
    }

    // Viewport "No Content" Helper Overlay
    if (SceneGraph::Get().GetRootNodes().empty()) {
        const auto& pal = Theme::GetPalette();
        ImVec2 centerPos = ImVec2(cursorPos.x + viewportAvail.x * 0.5f, cursorPos.y + viewportAvail.y * 0.5f);
        
        ImGui::SetCursorScreenPos(ImVec2(centerPos.x - 220.0f, centerPos.y - 45.0f));
        ImGui::BeginGroup();
        
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(pal.bgElevated.x, pal.bgElevated.y, pal.bgElevated.z, 0.85f));
        
        if (ImGui::BeginChild("##EmptyViewportHelper", ImVec2(440.0f, 90.0f), true, ImGuiWindowFlags_NoScrollbar)) {
            ImGui::SetCursorPosY(12.0f);
            ImGui::TextColored(pal.textSecondary, "    Drag 3D assets here or use Create > 3D Object");
            ImGui::Spacing();
            ImGui::SetCursorPosX(35.0f);
            
            if (ImGui::Button("+ Cube", ImVec2(100.0f, 26.0f))) {
                SceneNode node;
                node.id = SceneGraph::Get().GenerateNodeId();
                node.name = "Cube";
                node.type = SceneNodeType::Actor;
                node.meshPath = "Engine/DefaultCube";
                SceneGraph::Get().AddNode(node);
                ZeGFXAdapter::Get().CreateDefaultPrimitives();
            }
            ImGui::SameLine(0.0f, 12.0f);
            if (ImGui::Button("+ Sphere", ImVec2(100.0f, 26.0f))) {
                SceneNode node;
                node.id = SceneGraph::Get().GenerateNodeId();
                node.name = "Sphere";
                node.type = SceneNodeType::Actor;
                node.meshPath = "Engine/DefaultCube";
                SceneGraph::Get().AddNode(node);
            }
            ImGui::SameLine(0.0f, 12.0f);
            if (ImGui::Button("+ Plane", ImVec2(100.0f, 26.0f))) {
                SceneNode node;
                node.id = SceneGraph::Get().GenerateNodeId();
                node.name = "Plane";
                node.type = SceneNodeType::Actor;
                node.meshPath = "Engine/DefaultCube";
                SceneGraph::Get().AddNode(node);
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndGroup();
    }

    // Drag-and-drop target handling
    float viewMat[16], projMat[16];
    camera.GetViewMatrix(viewMat);
    camera.GetProjectionMatrix(viewportAvail.x / viewportAvail.y, projMat);
    Panels::ViewportDragDrop::Get().HandleDragDropTarget(cursorPos, viewportAvail, viewMat, projMat);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const auto& pal = Theme::GetPalette();

    // Render Speed Readout HUD Overlay when Fly Mode or Speed Tuning is Active
    if (camera.IsSpeedTuningActive() || camera.GetMode() == CameraMode::Fly) {
        char speedBuf[64];
        snprintf(speedBuf, sizeof(speedBuf), "Cam Speed: %.1f m/s", camera.GetMoveSpeed());
        float overlayY = cursorPos.y + Theme::Metrics::headerHeight + 12.0f;
        drawList->AddRectFilled(ImVec2(cursorPos.x + 16.0f, overlayY), ImVec2(cursorPos.x + 175.0f, overlayY + 26.0f), IM_COL32(15, 20, 28, 240), 4.0f);
        drawList->AddText(ImVec2(cursorPos.x + 24.0f, overlayY + 5.0f), IM_COL32(70, 180, 255, 255), speedBuf);
    }

    // === In-Viewport Control Strip (100% Solid Opaque Top Bar) ===
    float toolbarH = Theme::Metrics::headerHeight;
    {
        float toolbarY = cursorPos.y;
        float toolbarX = cursorPos.x;
        float toolbarW = viewportAvail.x;
        
        drawList->AddRectFilled(
            ImVec2(toolbarX, toolbarY),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            IM_COL32(18, 22, 28, 255));
        drawList->AddLine(
            ImVec2(toolbarX, toolbarY + toolbarH),
            ImVec2(toolbarX + toolbarW, toolbarY + toolbarH),
            IM_COL32(45, 52, 65, 255), 1.0f);
    }
    
    ImGui::SetCursorScreenPos(ImVec2(cursorPos.x + Theme::Metrics::panelLeftMargin, cursorPos.y + (toolbarH - 24.0f) * 0.5f));
    ImGui::BeginGroup();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(28, 34, 44, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(42, 50, 64, 255));

    // --- Group 1: Perspective / 1:2 / Show / Shading Mode ---
    const char* persNames[] = { "Perspective", "Top", "Front", "Side" };
    if (ImGui::Button(persNames[s_PerspectiveIdx])) ImGui::OpenPopup("ViewportPerspPopup");
    if (ImGui::BeginPopup("ViewportPerspPopup")) {
        if (ImGui::MenuItem("Perspective", nullptr, s_PerspectiveIdx == 0)) { s_PerspectiveIdx = 0; camera.ResetToDefault(); }
        if (ImGui::MenuItem("Top", nullptr, s_PerspectiveIdx == 1)) { s_PerspectiveIdx = 1; }
        if (ImGui::MenuItem("Front", nullptr, s_PerspectiveIdx == 2)) { s_PerspectiveIdx = 2; }
        if (ImGui::MenuItem("Side", nullptr, s_PerspectiveIdx == 3)) { s_PerspectiveIdx = 3; }
        ImGui::EndPopup();
    }

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    const char* qualNames[] = { "1:2", "1:1", "4K" };
    if (ImGui::Button(qualNames[s_QualityIdx])) ImGui::OpenPopup("ViewportQualPopup");
    if (ImGui::BeginPopup("ViewportQualPopup")) {
        if (ImGui::MenuItem("1:2 (Half Res)", nullptr, s_QualityIdx == 0)) {
            s_QualityIdx = 0;
            EditorState::Get().settings.dsrScale = 0.5f;
        }
        if (ImGui::MenuItem("1:1 (Full Res)", nullptr, s_QualityIdx == 1)) {
            s_QualityIdx = 1;
            EditorState::Get().settings.dsrScale = 1.0f;
        }
        if (ImGui::MenuItem("4K (Ultra HD)", nullptr, s_QualityIdx == 2)) {
            s_QualityIdx = 2;
            EditorState::Get().settings.dsrScale = 2.0f;
        }
        ImGui::EndPopup();
    }

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (ImGui::Button("Show")) ImGui::OpenPopup("ViewportShowPopup");
    if (ImGui::BeginPopup("ViewportShowPopup")) {
        bool showGrid = (s_ShowFlags & 1) != 0;
        bool showLights = (s_ShowFlags & 2) != 0;
        bool showGeo = (s_ShowFlags & 4) != 0;
        if (ImGui::Checkbox("Grid", &showGrid)) s_ShowFlags ^= 1;
        if (ImGui::Checkbox("Light Icons", &showLights)) s_ShowFlags ^= 2;
        if (ImGui::Checkbox("Geometry Outlines", &showGeo)) s_ShowFlags ^= 4;
        ImGui::EndPopup();
    }

    static int s_RenderPassMode = 0;
    const char* renderPassNames[] = { "Lit (PBR)", "Unlit", "Depth Buffer", "World Normals", "Roughness & Metallic", "Volumetric Fog Grid", "Cascaded Shadow Atlas" };

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (ImGui::Button(renderPassNames[s_RenderPassMode])) ImGui::OpenPopup("ViewportRenderPassPopup");
    if (ImGui::BeginPopup("ViewportRenderPassPopup")) {
        for (int r = 0; r < 7; ++r) {
            if (ImGui::MenuItem(renderPassNames[r], nullptr, s_RenderPassMode == r)) {
                s_RenderPassMode = r;
            }
        }
        ImGui::EndPopup();
    }

    // --- Right-Aligned Live Engine Status Readouts (Unlocked Dynamic FPS) ---
    const auto& stats = EditorState::Get().stats;
    uint32_t entCount = (uint32_t)SceneGraph::Get().GetRootNodes().size();
    
    float realFps = (io.DeltaTime > 0.00001f) ? (1.0f / io.DeltaTime) : 60.0f;
    float realMs = io.DeltaTime * 1000.0f;

    char fpsBuf[32]; snprintf(fpsBuf, sizeof(fpsBuf), "FPS: %.1f", realFps);
    char msBuf[32]; snprintf(msBuf, sizeof(msBuf), "Frame: %.2f ms", realMs);
    char drrBuf[32]; snprintf(drrBuf, sizeof(drrBuf), "DRR: %s", qualNames[s_QualityIdx]);
    char wpBuf[48]; snprintf(wpBuf, sizeof(wpBuf), "WP: 2 Cells");
    char entBuf[32]; snprintf(entBuf, sizeof(entBuf), "Entities: %u", entCount);

    const float pillPaddingX = 16.0f;
    const float gap = Theme::Metrics::intraGroupGap;
    float actualPillsWidth = 0.0f;
    actualPillsWidth += ImGui::CalcTextSize(stats.apiTag.c_str()).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(fpsBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(msBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(drrBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(wpBuf).x + pillPaddingX + gap;
    actualPillsWidth += ImGui::CalcTextSize(entBuf).x + pillPaddingX;

    float rightMargin = 20.0f;
    float pillsStart = viewportAvail.x - actualPillsWidth - rightMargin;
    float currentX = ImGui::GetCursorPosX() - cursorPos.x;

    if (pillsStart > currentX + 16.0f) {
        ImGui::SameLine(pillsStart);
    } else {
        ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    }

    RenderStatusPill(stats.apiTag.c_str(), pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(fpsBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(msBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(drrBuf, pal.accent);
    ImGui::SameLine(0.0f, gap);

    RenderStatusPill(wpBuf, pal.textSecondary);
    ImGui::SameLine(0.0f, gap);

    if (ImGui::Button(entBuf)) ImGui::OpenPopup("EntitiesPopup");
    if (ImGui::BeginPopup("EntitiesPopup")) {
        ImGui::Text("Active Scene Entities: %u", entCount);
        ImGui::Separator();
        ImGui::MenuItem("Select All Entities");
        ImGui::MenuItem("Hide Unselected Entities");
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    // Render 3D Floor Grid, Sun Light & Gizmo Overlays clipped strictly BELOW top bar
    drawList->PushClipRect(
        ImVec2(cursorPos.x, cursorPos.y + toolbarH),
        ImVec2(cursorPos.x + viewportAvail.x, cursorPos.y + viewportAvail.y),
        true
    );

    // 1. Render 3D Overlays, Grids, Orientation Triad, Isolation Banner
    Panels::RenderViewport3DOverlays(drawList, cursorPos, viewportAvail, s_ShowFlags);

    // 2. Render Transform Gizmos
    Panels::RenderViewportGizmos(drawList, cursorPos, viewportAvail);

    // 3. Handle Mouse Marquee Drag Box Selection
    if (isHovered && camera.GetMode() == CameraMode::Idle && !ImGuizmo::IsOver() && !ImGuizmo::IsUsing()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            Panels::ViewportSelection::Get().StartMarquee(io.MousePos);
        } else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            Panels::ViewportSelection::Get().UpdateMarquee(io.MousePos);
        } else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && Panels::ViewportSelection::Get().IsMarqueeActive()) {
            Panels::ViewportSelection::Get().EndMarquee(cursorPos, viewportAvail, viewMat, projMat, io.KeyShift, io.KeyCtrl);
        }

        // Right Click 3D Context Menu
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) && !io.KeyAlt) {
            Panels::RaycastHit hit;
            std::string hitNode = "";
            if (Panels::ViewportPicker::Get().PickNode(io.MousePos, cursorPos, viewportAvail, viewMat, projMat, hit)) {
                hitNode = hit.nodeName;
            }
            Panels::ViewportContextMenu::Get().OpenMenu(hitNode);
        }
    }

    // 4. Render Marquee Drag Box & Measurement Tools
    Panels::ViewportSelection::Get().RenderMarquee(drawList);
    Panels::ViewportMeasurement::Get().Render(drawList, cursorPos, viewportAvail, viewMat, projMat);

    drawList->PopClipRect();

    // 5. Render 3D Context Menu Popup
    Panels::ViewportContextMenu::Get().Render();

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace EngineEditor
