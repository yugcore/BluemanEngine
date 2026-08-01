#include "WindowLayout.h"
#include "core/EditorState.h"
#include <imgui_internal.h>

namespace EngineEditor::Layout {

static WorkspaceMode s_LastBuiltWorkspace = static_cast<WorkspaceMode>(-1);
static bool s_ResetRequested = false;

void RequestLayoutReset() {
    s_ResetRequested = true;
}

void SetupDefaultLayout(ImGuiID dockspaceId, const DockspaceBounds& bounds, bool forceRebuild) {
    WorkspaceMode mode = EditorState::Get().activeWorkspace;

    bool needRebuild = forceRebuild || s_ResetRequested || (s_LastBuiltWorkspace != mode);

    if (!needRebuild) {
        ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspaceId);
        if (node == nullptr || node->IsEmpty()) {
            needRebuild = true;
        }
    }

    if (!needRebuild) return;

    s_LastBuiltWorkspace = mode;
    s_ResetRequested = false;

    ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoWindowMenuButton;
    ImGui::DockBuilderRemoveNodeDockedWindows(dockspaceId, true);
    ImGui::DockBuilderRemoveNode(dockspaceId);
    ImGui::DockBuilderAddNode(dockspaceId, dockspaceFlags | ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspaceId, bounds.size);


    if (mode == WorkspaceMode::Editor) {
        ImGuiID dockMain = dockspaceId;
        ImGuiID dockRight = 0;
        ImGuiID dockBottom = 0;
        ImGuiID dockViewport = 0;
        ImGuiID dockRightTop = 0;
        ImGuiID dockRightBottom = 0;

        ImGuiID dockBottomLeft = 0;
        ImGuiID dockBottomRight = 0;

        // Split 1: Bottom dock - 30% height, full width at bottom
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.30f, &dockBottom, &dockMain);

        // Split 1b: Split dockBottom into dominant Content Browser (Left 75%) and Output Log (Right 25%)
        ImGui::DockBuilderSplitNode(dockBottom, ImGuiDir_Left, 0.75f, &dockBottomLeft, &dockBottomRight);

        // Split 2: Top-right dock (Outliner & Details) - 25% width of top area
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.25f, &dockRight, &dockViewport);

        // Split 3: Right dock split vertically into Outliner (Top 45%) and Details (Bottom 55%)
        ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.55f, &dockRightBottom, &dockRightTop);

        // Assign Windows to Dock Nodes
        // Always Visible
        ImGui::DockBuilderDockWindow("Viewport", dockViewport);
        ImGui::DockBuilderDockWindow("Content Browser", dockBottomLeft);
        ImGui::DockBuilderDockWindow("Outliner", dockRightTop);
        ImGui::DockBuilderDockWindow("Details", dockRightBottom);

        // Hidden & Context-Sensitive Workspaces (Docked centrally or in side/bottom tabs)
        ImGui::DockBuilderDockWindow("Material Editor", dockViewport);
        ImGui::DockBuilderDockWindow("Blueprint / Visual Script", dockViewport);
        ImGui::DockBuilderDockWindow("Animation Workspace", dockViewport);
        ImGui::DockBuilderDockWindow("Shader Studio", dockViewport);
        ImGui::DockBuilderDockWindow("Mesh Studio", dockViewport);
        ImGui::DockBuilderDockWindow("Texture Viewer", dockViewport);

        ImGui::DockBuilderDockWindow("Output Log", dockBottomRight);
        ImGui::DockBuilderDockWindow("Profiler", dockBottomRight);
        ImGui::DockBuilderDockWindow("RenderDoc Integrator", dockBottomRight);
        ImGui::DockBuilderDockWindow("GPU Debugger", dockBottomRight);
        ImGui::DockBuilderDockWindow("Audio Mixer", dockBottomRight);
        ImGui::DockBuilderDockWindow("Sequencer / Timeline", dockBottomLeft);
        ImGui::DockBuilderDockWindow("Console Variables", dockBottomRight);

        ImGui::DockBuilderDockWindow("Object Palette", dockRightTop);
        ImGui::DockBuilderDockWindow("Asset Registry", dockRightTop);
        ImGui::DockBuilderDockWindow("Package Manager", dockRightTop);
        ImGui::DockBuilderDockWindow("Plugin Manager", dockRightTop);
        ImGui::DockBuilderDockWindow("Localization", dockRightTop);
        ImGui::DockBuilderDockWindow("Navigation Builder", dockRightTop);
        ImGui::DockBuilderDockWindow("Light Baking", dockRightTop);
        ImGui::DockBuilderDockWindow("Render Control Strip", dockRightBottom);
    }
    else if (mode == WorkspaceMode::Codebase) {
        ImGuiID dockMain = dockspaceId;
        ImGuiID dockLeft = 0;
        ImGuiID dockRight = 0;
        ImGuiID dockBottom = 0;
        ImGuiID dockCenter = 0;

        // Split 1: Bottom dock (Terminal/Build Output/Git/Problems/Debug Console) - 25% height
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25f, &dockBottom, &dockMain);

        // Split 2: Left dock (Project Explorer) - 20% width
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20f, &dockLeft, &dockMain);

        // Split 3: Right dock (Symbols / References / Outline / Call Hierarchy) - 22% width
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.22f, &dockRight, &dockCenter);

        // Assign Windows to Dock Nodes
        ImGui::DockBuilderDockWindow("Project Explorer", dockLeft);
        ImGui::DockBuilderDockWindow("Code Editor", dockCenter);
        ImGui::DockBuilderDockWindow("API Library", dockRight);
        ImGui::DockBuilderDockWindow("Terminal", dockBottom);
    }
    else if (mode == WorkspaceMode::Run) {
        ImGuiID dockMain = dockspaceId;
        ImGuiID dockBottom = 0;
        ImGuiID dockViewport = 0;

        // Split 1: Bottom dock (Logs/Breakpoints/Variables/Profiler/RenderDoc) - 25% height
        ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25f, &dockBottom, &dockViewport);

        // Assign Windows to Dock Nodes
        ImGui::DockBuilderDockWindow("Running Game", dockViewport);
        ImGui::DockBuilderDockWindow("Logs", dockBottom);
    }

    ImGui::DockBuilderFinish(dockspaceId);
}

} // namespace EngineEditor::Layout
