#include "MenuBar.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"
#include "panels/Chrome/CustomTitleBar.h"
#include "panels/Chrome/Toolbar.h"
#include "panels/Chrome/StatusBar.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include <imgui.h>

namespace EngineEditor {

// Shared menu content rendering — used by both standalone and inline modes
void RenderMenuBarContents() {
    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, pal.textDisabled);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));   // idle top-level item: no box
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, pal.bgHeader);   // hovered item / open submenu
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, pal.accent);   // clicked/held
    ImGui::PushStyleColor(ImGuiCol_Separator, pal.borderSubtle);
    ImGui::PushStyleColor(ImGuiCol_Border, pal.borderSubtle);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::groupGap, Theme::Metrics::intraGroupGap));   // gaps between top-level menus
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::sectionIndent, 6.0f)); // dropdown inner padding
    ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New Project Wizard...", "Ctrl+Shift+N")) EditorState::Get().showProjectWizardModal = true;
        if (ImGui::MenuItem("New Scene", "Ctrl+N")) Logger::Get().Info("[Menu] File > New Scene created.");
        if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) Logger::Get().Info("[Menu] File > Open Scene dialog opened.");
        if (ImGui::MenuItem("Save", "Ctrl+S")) Logger::Get().Info("[Menu] File > Save Scene completed.");
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) Logger::Get().Info("[Menu] File > Save As dialog opened.");
        ImGui::Separator();
        if (ImGui::MenuItem("Import Assets...", "Ctrl+I")) Logger::Get().Info("[Menu] File > Import Assets dialog opened.");
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) Logger::Get().Info("[Menu] File > Exit selected.");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Undo", "Ctrl+Z")) Logger::Get().Info("[Menu] Edit > Undo clicked.");
        if (ImGui::MenuItem("Redo", "Ctrl+Y")) Logger::Get().Info("[Menu] Edit > Redo clicked.");
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "Ctrl+X")) Logger::Get().Info("[Menu] Edit > Cut clicked.");
        if (ImGui::MenuItem("Copy", "Ctrl+C")) Logger::Get().Info("[Menu] Edit > Copy clicked.");
        if (ImGui::MenuItem("Paste", "Ctrl+V")) Logger::Get().Info("[Menu] Edit > Paste clicked.");
        if (ImGui::MenuItem("Duplicate", "Ctrl+D")) Logger::Get().Info("[Menu] Edit > Duplicate clicked.");
        if (ImGui::MenuItem("Delete", "Del")) Logger::Get().Info("[Menu] Edit > Delete clicked.");
        ImGui::Separator();
        if (ImGui::MenuItem("Project Settings...")) EditorState::Get().showProjectSettingsModal = true;
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Create")) {
        if (ImGui::MenuItem("Empty GameObject")) Logger::Get().Info("[Menu] Create > Empty GameObject");
        if (ImGui::BeginMenu("3D Object")) {
            if (ImGui::MenuItem("Cube")) Logger::Get().Info("[Menu] Create > Cube");
            if (ImGui::MenuItem("Sphere")) Logger::Get().Info("[Menu] Create > Sphere");
            if (ImGui::MenuItem("Cylinder")) Logger::Get().Info("[Menu] Create > Cylinder");
            if (ImGui::MenuItem("Plane")) Logger::Get().Info("[Menu] Create > Plane");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Light")) {
            if (ImGui::MenuItem("Directional Light")) Logger::Get().Info("[Menu] Create > Directional Light");
            if (ImGui::MenuItem("Point Light")) Logger::Get().Info("[Menu] Create > Point Light");
            if (ImGui::MenuItem("Spot Light")) Logger::Get().Info("[Menu] Create > Spot Light");
            if (ImGui::MenuItem("SkyAtmosphere")) Logger::Get().Info("[Menu] Create > SkyAtmosphere");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Audio")) {
            if (ImGui::MenuItem("Audio Source")) Logger::Get().Info("[Menu] Create > Audio Source");
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Window")) {
        auto& state = EditorState::Get();

        ImGui::MenuItem("Object Palette", nullptr, &state.showObjectPalettePanel);
        ImGui::MenuItem("Mesh Studio", nullptr, &state.showMeshStudioPanel);
        ImGui::MenuItem("Shader Studio", nullptr, &state.showShaderStudioPanel);
        ImGui::MenuItem("Texture Viewer", nullptr, &state.showTextureViewerPanel);
        ImGui::Separator();
        ImGui::MenuItem("Content Browser", nullptr, &state.settings.showContentBrowser);
        ImGui::MenuItem("Output Log", nullptr, &state.settings.showOutputLog);
        ImGui::MenuItem("Render Control Strip", nullptr, &state.settings.showRenderControlStrip);
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Layout")) {
            Layout::RequestLayoutReset();
            Logger::Get().Info("[Menu] Window > Reset Layout executed.");
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools")) {
        if (ImGui::MenuItem("Material Editor")) Logger::Get().Info("[Menu] Tools > Material Editor");
        if (ImGui::MenuItem("Level Sequence Editor")) Logger::Get().Info("[Menu] Tools > Level Sequence Editor");
        if (ImGui::MenuItem("Animation Editor")) Logger::Get().Info("[Menu] Tools > Animation Editor");
        ImGui::Separator();
        if (ImGui::MenuItem("Shader Compiler")) Logger::Get().Info("[Menu] Tools > Shader Compiler");
        if (ImGui::MenuItem("Asset Validator")) Logger::Get().Info("[Menu] Tools > Asset Validator");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Build")) {
        if (ImGui::MenuItem("Build Geometry")) Logger::Get().Info("[Menu] Build > Geometry");
        if (ImGui::MenuItem("Build Lighting (DXR)")) Logger::Get().Info("[Menu] Build > Lighting");
        if (ImGui::MenuItem("Build Navigation Grid")) Logger::Get().Info("[Menu] Build > Navigation");
        ImGui::Separator();
        if (ImGui::MenuItem("Build All", "Ctrl+Shift+B")) Logger::Get().Info("[Menu] Build All");
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Documentation")) Logger::Get().Info("[Menu] Help > Documentation");
        if (ImGui::MenuItem("About Blueman Engine")) Logger::Get().Info("[Menu] Help > About");
        ImGui::EndMenu();
    }

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(8);
}

// Standalone menu bar (no longer called when using unified title bar)
void RenderMenuBar() {
    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, pal.bgBase);
    if (ImGui::BeginMainMenuBar()) {
        RenderMenuBarContents();
        ImGui::EndMainMenuBar();
    }
    ImGui::PopStyleColor();
}

} // namespace EngineEditor