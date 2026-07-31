#include "Toolbar.h"
#include "CustomTitleBar.h"
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

// Toolbar layout constants
static constexpr float kToolbarHeight = 36.0f;
static constexpr float kButtonHeight  = Theme::Metrics::rowHeight;
static constexpr float kSeparatorSpacing = Theme::Metrics::sectionIndent;

float GetToolbarTotalHeight() {
    return kToolbarHeight;
}

// --- Reusable toolbar button with optional active state ---
static bool ToolbarButton(const char* label, const char* tooltip, bool isActive = false,
                          float width = 0.0f) {
    const auto& pal = Theme::GetPalette();
    if (isActive) {
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
    bool clicked = ImGui::Button(label, ImVec2(width, kButtonHeight));
    if (ImGui::IsItemHovered() && tooltip[0] != '\0') {
        ImGui::SetTooltip("%s", tooltip);
    }
    ImGui::PopStyleColor(4);
    return clicked;
}

// --- Vertical separator with spacing ---
static void ToolbarSeparator() {
    ImGui::SameLine(0.0f, kSeparatorSpacing);
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine(0.0f, kSeparatorSpacing);
}

void RenderToolbar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const auto& pal = Theme::GetPalette();

    // Position toolbar below the title bar
    float titleBarBottom = viewport->Pos.y + GetTitleBarTotalHeight();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, titleBarBottom));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kToolbarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::panelLeftMargin, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgPanel);

    if (ImGui::Begin("##ToolbarWindow", nullptr, windowFlags)) {

        // === Group 1: Save / Undo / Redo ===
        if (ToolbarButton("Save", "Save Scene (Ctrl+S)", false, 50.0f)) Logger::Get().Info("[Toolbar] Quick Save executed.");
        ImGui::SameLine();
        if (ToolbarButton("Undo", "Undo Action (Ctrl+Z)", false, 50.0f)) CommandStack::Get().Undo();
        ImGui::SameLine();
        if (ToolbarButton("Redo", "Redo Action (Ctrl+Y)", false, 50.0f)) CommandStack::Get().Redo();

        ToolbarSeparator();

        // === Group 2: Import / Add dropdowns ===
        if (ToolbarButton("Import \xE2\x96\xBC", "Import External Assets (Ctrl+I)", false, 72.0f))
            ImGui::OpenPopup("ImportPopup");
        if (ImGui::BeginPopup("ImportPopup")) {
            if (ImGui::MenuItem("Import 3D Mesh...")) Logger::Get().Info("[Toolbar] Import Mesh selected");
            if (ImGui::MenuItem("Import Texture...")) Logger::Get().Info("[Toolbar] Import Texture selected");
            if (ImGui::MenuItem("Import Audio...")) Logger::Get().Info("[Toolbar] Import Audio selected");
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ToolbarButton("+ Add \xE2\x96\xBC", "Add Actor or Component", false, 68.0f))
            ImGui::OpenPopup("AddPopup");
        if (ImGui::BeginPopup("AddPopup")) {
            if (ImGui::MenuItem("Add Actor")) Logger::Get().Info("[Toolbar] Add Actor selected");
            if (ImGui::MenuItem("Add Component")) Logger::Get().Info("[Toolbar] Add Component selected");
            if (ImGui::MenuItem("Add Light")) Logger::Get().Info("[Toolbar] Add Light selected");
            ImGui::EndPopup();
        }

        ToolbarSeparator();

        // === Group 3: Transform Tools ===
        auto& gizmoOp = EditorState::Get().gizmoOp;
        bool isMove = (gizmoOp == GizmoOperation::Translate);
        bool isRotate = (gizmoOp == GizmoOperation::Rotate);
        bool isScale = (gizmoOp == GizmoOperation::Scale);

        if (ToolbarButton("Select", "Select Tool (Q)", false, 55.0f)) gizmoOp = GizmoOperation::Translate;
        ImGui::SameLine();
        if (ToolbarButton("Move", "Move Tool (W)", isMove, 50.0f)) gizmoOp = GizmoOperation::Translate;
        ImGui::SameLine();
        if (ToolbarButton("Rotate", "Rotate Tool (E)", isRotate, 55.0f)) gizmoOp = GizmoOperation::Rotate;
        ImGui::SameLine();
        if (ToolbarButton("Scale", "Scale Tool (R)", isScale, 50.0f)) gizmoOp = GizmoOperation::Scale;

        ToolbarSeparator();

        // === Group 4: Snap Toggle ===
        {
            char snapBuf[32];
            snprintf(snapBuf, sizeof(snapBuf), "Snap %.2f", snapValue);
            if (ToolbarButton(snapBuf, "Toggle Transform Grid Snapping", snapEnabled, 82.0f))
                snapEnabled = !snapEnabled;
        }

        ToolbarSeparator();

        // === Group 5: Play / Frame / Stop (neutral monochrome) ===
        if (ToolbarButton("\xE2\x96\xB6 Play", "Start Simulation / Play", false, 65.0f))
            Logger::Get().Info("[Toolbar] Play mode started.");

        ImGui::SameLine();
        if (ToolbarButton("Frame", "Single Frame Step", false, 55.0f))
            Logger::Get().Info("[Toolbar] Single frame step.");

        ImGui::SameLine();
        if (ToolbarButton("Stop", "Stop Simulation", false, 50.0f))
            Logger::Get().Info("[Toolbar] Simulation stopped.");

        ToolbarSeparator();

        // === Group 6: Build dropdown ===
        if (ToolbarButton("Build \xE2\x96\xBC", "Build Geometry and Lighting", false, 68.0f))
            ImGui::OpenPopup("BuildPopup");
        if (ImGui::BeginPopup("BuildPopup")) {
            if (ImGui::MenuItem("Quick Build")) Logger::Get().Info("[Toolbar] Quick Build started");
            if (ImGui::MenuItem("Production Build")) Logger::Get().Info("[Toolbar] Production Build started");
            ImGui::EndPopup();
        }

        // === Right-aligned controls ===
        float rightControlsWidth = 260.0f;
        float rightStart = ImGui::GetWindowWidth() - rightControlsWidth;
        if (rightStart > ImGui::GetCursorPosX()) {
            ImGui::SameLine(rightStart);
        } else {
            ImGui::SameLine();
        }

        const char* perspectiveOptions[] = { "Perspective \xE2\x96\xBC", "Top \xE2\x96\xBC", "Front \xE2\x96\xBC", "Side \xE2\x96\xBC" };
        if (ToolbarButton(perspectiveOptions[currentPerspective], "Camera Perspective", false, 100.0f))
            ImGui::OpenPopup("PerspectivePopup");
        if (ImGui::BeginPopup("PerspectivePopup")) {
            if (ImGui::MenuItem("Perspective", nullptr, currentPerspective == 0)) currentPerspective = 0;
            if (ImGui::MenuItem("Top Ortho", nullptr, currentPerspective == 1)) currentPerspective = 1;
            if (ImGui::MenuItem("Front Ortho", nullptr, currentPerspective == 2)) currentPerspective = 2;
            if (ImGui::MenuItem("Side Ortho", nullptr, currentPerspective == 3)) currentPerspective = 3;
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        const char* viewModes[] = { "Lit \xE2\x96\xBC", "Unlit \xE2\x96\xBC", "Wire \xE2\x96\xBC", "Light \xE2\x96\xBC" };
        if (ToolbarButton(viewModes[currentViewMode], "View Mode", false, 60.0f))
            ImGui::OpenPopup("ViewModePopup");
        if (ImGui::BeginPopup("ViewModePopup")) {
            if (ImGui::MenuItem("Lit", nullptr, currentViewMode == 0)) currentViewMode = 0;
            if (ImGui::MenuItem("Unlit", nullptr, currentViewMode == 1)) currentViewMode = 1;
            if (ImGui::MenuItem("Wireframe", nullptr, currentViewMode == 2)) currentViewMode = 2;
            if (ImGui::MenuItem("Lighting Only", nullptr, currentViewMode == 3)) currentViewMode = 3;
            ImGui::EndPopup();
        }

        ImGui::SameLine();
        if (ToolbarButton("Settings", "Editor Settings", false, 65.0f))
            Logger::Get().Info("[Toolbar] Editor Settings dialog opened.");
    }
    ImGui::End();

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor