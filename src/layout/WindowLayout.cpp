#include "WindowLayout.h"
#include <imgui_internal.h>

namespace EngineEditor::Layout {

static bool s_DefaultLayoutBuilt = false;

void SetupDefaultLayout(ImGuiID dockspaceId, const DockspaceBounds& bounds, bool forceRebuild) {
    if (!forceRebuild && s_DefaultLayoutBuilt) return;
    if (ImGui::DockBuilderGetNode(dockspaceId) != nullptr && !ImGui::DockBuilderGetNode(dockspaceId)->IsEmpty()) return;

    s_DefaultLayoutBuilt = true;

    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, bounds.size);

    ImGuiID dockMain = dockspaceId;
    ImGuiID dockLeft = 0;
    ImGuiID dockRight = 0;
    ImGuiID dockBottom = 0;
    ImGuiID dockViewport = 0;
    ImGuiID dockRightTop = 0;
    ImGuiID dockRightBottom = 0;

    // Split 1: Left panel (Content Browser) - 18% for less cramped viewport
    ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.18f, &dockLeft, &dockMain);
    
    // Split 2: Right panel (Outliner / Details) - 22%
    ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.22f, &dockRight, &dockMain);
    
    // Split 3: Bottom panel (Output Log) - 22%
    ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.22f, &dockBottom, &dockViewport);

    // Split 4: Right panel split vertically into Outliner (Top 45%) and Details (Bottom 55%)
    ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.55f, &dockRightBottom, &dockRightTop);

    // Assign Windows to Dock Nodes (Exact window titles matching imgui.ini)
    ImGui::DockBuilderDockWindow("Content Browser", dockLeft);
    ImGui::DockBuilderDockWindow("Viewport", dockViewport);
    ImGui::DockBuilderDockWindow("Outliner", dockRightTop);
    ImGui::DockBuilderDockWindow("Details", dockRightBottom);
    ImGui::DockBuilderDockWindow("Render Control Strip", dockRightBottom);
    ImGui::DockBuilderDockWindow("Output Log", dockBottom);

    ImGui::DockBuilderFinish(dockspaceId);
}

} // namespace EngineEditor::Layout
