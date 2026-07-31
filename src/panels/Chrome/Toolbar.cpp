#include "Toolbar.h"
#include "CustomTitleBar.h"
#include "WorkspaceBar.h"
#include "core/EditorState.h"
#include "core/CommandStack.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <cstdio>

namespace EngineEditor {

static bool snapEnabled = true;
static float snapValue = 0.50f;
static int currentViewMode = 0;
static int currentPerspective = 0;

static constexpr float kToolbarHeight = 42.0f;
static constexpr float kButtonHeight  = 24.0f;

float GetToolbarTotalHeight() {
    return kToolbarHeight;
}

// --- Reusable toolbar button ---
static bool ToolbarButton(const char* label, const char* tooltip, bool isActive = false,
                          float width = 0.0f, bool isDisabled = false) {
    const auto& pal = Theme::GetPalette();
    if (isDisabled) {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textDisabled);
    } else if (isActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.accentHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accentActive);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
    }

    bool clicked = ImGui::Button(label, ImVec2(width, 0.0f));
    if (ImGui::IsItemHovered() && tooltip[0] != '\0') {
        ImGui::SetTooltip("%s", tooltip);
    }
    ImGui::PopStyleColor(4);
    return clicked;
}

// --- Vertical separator with 24px groupGap cluster spacing ---
static void ToolbarSeparator() {
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
}

void RenderToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const auto& pal = Theme::GetPalette();
    auto& state = EditorState::Get();

    // Position toolbar below the workspace bar
    float topOffset = GetTitleBarTotalHeight() + GetWorkspaceBarTotalHeight();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + topOffset));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kToolbarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::panelLeftMargin, 7.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::intraGroupGap, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgPanel);

    if (ImGui::Begin("##ToolbarWindow", nullptr, windowFlags)) {

        // ====================================================================
        // WORKSPACE MODE: EDITOR
        // ====================================================================
        if (state.activeWorkspace == WorkspaceMode::Editor) {

            // File Actions
            if (ToolbarButton("Save", "Save Scene (Ctrl+S)", false, 60.0f)) Logger::Get().Info("[Toolbar] Quick Save executed.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Undo", "Undo Action (Ctrl+Z)", false, 60.0f)) CommandStack::Get().Undo();
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Redo", "Redo Action (Ctrl+Y)", false, 60.0f)) CommandStack::Get().Redo();

            ToolbarSeparator();

            // Import / Add
            // Import / Add
            if (ToolbarButton("Import", "Import External Assets", false, 0.0f)) ImGui::OpenPopup("ImportPopup");
            if (ImGui::BeginPopup("ImportPopup")) {
                if (ImGui::MenuItem("Import 3D Mesh...")) Logger::Get().Info("[Toolbar] Import Mesh");
                if (ImGui::MenuItem("Import Texture...")) Logger::Get().Info("[Toolbar] Import Texture");
                ImGui::EndPopup();
            }

            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("+ Add", "Add Actor or Component", false, 0.0f)) ImGui::OpenPopup("AddPopup");
            if (ImGui::BeginPopup("AddPopup")) {
                if (ImGui::MenuItem("Add Actor")) Logger::Get().Info("[Toolbar] Add Actor");
                if (ImGui::MenuItem("Add Light")) Logger::Get().Info("[Toolbar] Add Light");
                ImGui::EndPopup();
            }

            ToolbarSeparator();

            // Selection & Transform Tools
            auto& gizmoOp = state.gizmoOp;
            bool isMove = (gizmoOp == GizmoOperation::Translate);
            bool isRotate = (gizmoOp == GizmoOperation::Rotate);
            bool isScale = (gizmoOp == GizmoOperation::Scale);

            if (ToolbarButton("Select", "Select Tool (Q)", false, 0.0f)) gizmoOp = GizmoOperation::Translate;
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Move", "Move Tool (W)", isMove, 0.0f)) gizmoOp = GizmoOperation::Translate;
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Rotate", "Rotate Tool (E)", isRotate, 0.0f)) gizmoOp = GizmoOperation::Rotate;
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Scale", "Scale Tool (R)", isScale, 0.0f)) gizmoOp = GizmoOperation::Scale;

            ToolbarSeparator();

            // Snapping
            {
                char snapBuf[32];
                snprintf(snapBuf, sizeof(snapBuf), "Snap %.2f", snapValue);
                if (ToolbarButton(snapBuf, "Toggle Transform Grid Snapping", snapEnabled, 0.0f))
                    snapEnabled = !snapEnabled;
            }

            ToolbarSeparator();

            // Simulation
            if (ToolbarButton("Play", "Start Simulation / Play", false, 0.0f)) Logger::Get().Info("[Toolbar] Play started.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Frame", "Single Frame Step", false, 0.0f)) Logger::Get().Info("[Toolbar] Frame step.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Stop", "Stop Simulation", false, 0.0f)) Logger::Get().Info("[Toolbar] Simulation stopped.");

            ToolbarSeparator();

            // Build
            if (ToolbarButton("Build", "Build Geometry and Lighting", false, 0.0f)) ImGui::OpenPopup("BuildPopup");
            if (ImGui::BeginPopup("BuildPopup")) {
                if (ImGui::MenuItem("Quick Build")) Logger::Get().Info("[Toolbar] Quick Build");
                if (ImGui::MenuItem("Production Build")) Logger::Get().Info("[Toolbar] Production Build");
                ImGui::EndPopup();
            }

        }
        // ====================================================================
        // WORKSPACE MODE: CODEBASE (Disabled Visual Stubs Only)
        // ====================================================================
        else if (state.activeWorkspace == WorkspaceMode::Codebase) {

            // File Actions
            ToolbarButton("Save File", "Save Active Document (Ctrl+S)", false, 0.0f);
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            ToolbarButton("Undo", "Undo Code Edit (Ctrl+Z)", false, 0.0f);
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            ToolbarButton("Redo", "Redo Code Edit (Ctrl+Y)", false, 0.0f);

            ToolbarSeparator();

            // Code Actions (Explanatory Disabled Stubs)
            ToolbarButton("Format", "Format Source Code (Coming soon: Zelyn/C++ auto-formatter)", false, 0.0f, true);
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            ToolbarButton("Find in Files", "Search Codebase (Coming soon: Global regex search)", false, 0.0f, true);

            ToolbarSeparator();

            // Build (Explanatory Disabled Stub)
            ToolbarButton("Build Code", "Build Workspace (Coming soon: Incremental compiler target integration)", false, 0.0f, true);

            ToolbarSeparator();

            // Git (Explanatory Disabled Stub)
            ToolbarButton("Git: main", "Git Branch (Coming soon: Embedded libgit2 integration)", false, 0.0f, true);

        }
        // ====================================================================
        // WORKSPACE MODE: RUN (Primary Simulation Controls & Disabled Stubs)
        // ====================================================================
        else if (state.activeWorkspace == WorkspaceMode::Run) {

            // Primary Simulation Controls
            if (ToolbarButton("Play", "Resume Game Execution", true, 0.0f)) Logger::Get().Info("[Run] Play clicked.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Pause", "Pause Execution", false, 0.0f)) Logger::Get().Info("[Run] Pause clicked.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Step", "Step Single Frame", false, 0.0f)) Logger::Get().Info("[Run] Step clicked.");
            ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
            if (ToolbarButton("Stop", "Stop Game Execution", false, 0.0f)) Logger::Get().Info("[Run] Stop clicked.");

            ToolbarSeparator();

            // Performance Preset (Disabled Visual Stub)
            ToolbarButton("Perf: High", "Performance Profile (Disabled Stub)", false, 0.0f, true);

            ToolbarSeparator();

            // Debugger Attacher (Disabled Visual Stub)
            ToolbarButton("Attach Debugger", "Attach Native Debugger (Disabled Stub)", false, 0.0f, true);

        }

        // ====================================================================
        // RIGHT-ALIGNED CONTROLS (Shared across workspaces)
        // ====================================================================
        float rightControlsWidth = 280.0f;
        float rightStart = ImGui::GetWindowWidth() - rightControlsWidth - Theme::Metrics::panelLeftMargin;
        if (rightStart > ImGui::GetCursorPosX()) {
            ImGui::SameLine(rightStart);
        } else {
            ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
        }

        const char* perspectiveOptions[] = { "Perspective", "Top", "Front", "Side" };
        if (ToolbarButton(perspectiveOptions[currentPerspective], "Camera Perspective", false, 0.0f)) {
            ImGui::OpenPopup("PerspectivePopup");
        }
        if (ImGui::BeginPopup("PerspectivePopup")) {
            if (ImGui::MenuItem("Perspective", nullptr, currentPerspective == 0)) currentPerspective = 0;
            if (ImGui::MenuItem("Top Ortho", nullptr, currentPerspective == 1)) currentPerspective = 1;
            if (ImGui::MenuItem("Front Ortho", nullptr, currentPerspective == 2)) currentPerspective = 2;
            if (ImGui::MenuItem("Side Ortho", nullptr, currentPerspective == 3)) currentPerspective = 3;
            ImGui::EndPopup();
        }

        ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
        const char* viewModes[] = { "Lit", "Unlit", "Wire", "Light" };
        if (ToolbarButton(viewModes[currentViewMode], "View Mode", false, 0.0f)) {
            ImGui::OpenPopup("ViewModePopup");
        }
        if (ImGui::BeginPopup("ViewModePopup")) {
            if (ImGui::MenuItem("Lit", nullptr, currentViewMode == 0)) currentViewMode = 0;
            if (ImGui::MenuItem("Unlit", nullptr, currentViewMode == 1)) currentViewMode = 1;
            if (ImGui::MenuItem("Wireframe", nullptr, currentViewMode == 2)) currentViewMode = 2;
            if (ImGui::MenuItem("Lighting Only", nullptr, currentViewMode == 3)) currentViewMode = 3;
            ImGui::EndPopup();
        }

        ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
        if (ToolbarButton("Settings", "Editor Settings", false, 75.0f))
            Logger::Get().Info("[Toolbar] Editor Settings dialog opened.");
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor