#include "Application.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"
#include "core/EditorState.h"
#include "engine/core/Logger.h"
#include "engine/scene/SceneGraph.h"
#include "engine/assets/AssetRegistry.h"
#include "engine/assets/BackgroundAssetCooker.h"
#include "core/CommandStack.h"
#include "render/SplashScreen.h"
#include "theme/Colors.h"

#include "panels/Chrome/CustomTitleBar.h"
#include "panels/Chrome/MenuBar.h"
#include "panels/Chrome/WorkspaceBar.h"
#include "panels/Chrome/Toolbar.h"
#include "panels/Chrome/StatusBar.h"

// Editor Panels
#include "panels/Viewport/ViewportPanel.h"
#include "panels/ImportProgress/ImportProgressModal.h"
#include "panels/ContentBrowser/ContentBrowserPanel.h"
#include "panels/Outliner/OutlinerPanel.h"
#include "panels/Details/DetailsPanel.h"
#include "panels/OutputLog/OutputLogPanel.h"
#include "panels/RenderControlStrip/RenderControlStripPanel.h"

// Codebase Panels
#include "panels/Codebase/ProjectExplorerPanel.h"
#include "panels/Codebase/CodeEditorPanel.h"
#include "panels/Codebase/CodeSymbolsPanel.h"
#include "panels/Codebase/CodeBottomDockPanel.h"

// Run Panels
#include "panels/Run/RunViewportPanel.h"
#include "panels/Run/RunBottomDockPanel.h"

// New Code-First Modules
#include "panels/ObjectPalette/ObjectPalettePanel.h"
#include "panels/MeshStudio/MeshStudioPanel.h"
#include "panels/ShaderStudio/ShaderStudioPanel.h"
#include "panels/TextureViewer/TextureViewerPanel.h"
#include "panels/ProjectWizard/ProjectWizardModal.h"
#include "panels/ProjectSettings/ProjectSettingsModal.h"
#include "panels/ContextPanels.h"

namespace EngineEditor {

void InitializeApplication() {
    // Explicitly trigger singleton creation in the correct order.
    // This ensures deterministic initialization and avoids the
    // C++ static initialization order fiasco.
    Logger::Get().Info("[Application] Subsystem initialization starting...");
    SceneGraph::Get();           // Scene hierarchy
    AssetRegistry::Get();        // Asset database
    CommandStack::Get();         // Undo/redo
    BackgroundAssetCooker::Get(); // Worker thread starts here
    Logger::Get().Info("[Application] All editor subsystems initialized.");
}

void RenderApplicationLayout() {
    auto& state = EditorState::Get();

    // Render Unreal-style Splash Screen if active
    if (SplashScreen::Get().IsActive()) {
        RenderSplashScreenUI();
        return;
    }

    // 1. Render unified title bar (includes menu bar inline)
    RenderCustomTitleBar();

    // 2. Render workspace switcher bar (Editor | Codebase | Run)
    RenderWorkspaceBar();

    // 3. Render workspace-aware toolbar below switcher bar
    RenderToolbar();

    // 4. Calculate Dockspace Host Bounds
    float topOffset = GetTitleBarTotalHeight() + GetWorkspaceBarTotalHeight() + GetToolbarTotalHeight();
    Layout::DockspaceBounds bounds = Layout::CalculateDockspaceBounds(topOffset, GetStatusBarTotalHeight());
    ImGuiID dockspaceId = Layout::RenderDockspaceHost(bounds);

    // 5. Setup Docking Layout for active workspace
    Layout::SetupDefaultLayout(dockspaceId, bounds);

    // 6. Render active workspace panels
    WorkspaceMode mode = state.activeWorkspace;

    if (mode == WorkspaceMode::Editor) {
        // Always Visible Panels (Default)
        if (state.showViewportPanel) RenderViewportPanel(&state.showViewportPanel);
        if (state.showOutlinerPanel) RenderOutlinerPanel(&state.showOutlinerPanel);
        if (state.settings.showContentBrowser) RenderContentBrowserPanel(&state.settings.showContentBrowser);
        if (state.showDetailsPanel) RenderDetailsPanel(&state.showDetailsPanel);

        // Hidden by Default Panels
        if (state.showObjectPalettePanel) RenderObjectPalettePanel(&state.showObjectPalettePanel);
        if (state.settings.showOutputLog) RenderOutputLogPanel(&state.settings.showOutputLog);
        if (state.settings.showRenderControlStrip) RenderRenderControlStripPanel(&state.settings.showRenderControlStrip);
        if (state.showMeshStudioPanel) RenderMeshStudioPanel(&state.showMeshStudioPanel);
        if (state.showShaderStudioPanel) RenderShaderStudioPanel(&state.showShaderStudioPanel);
        if (state.showTextureViewerPanel) RenderTextureViewerPanel(&state.showTextureViewerPanel);
        if (state.showProfilerPanel) RenderProfilerPanel(&state.showProfilerPanel);
        if (state.showRenderDocPanel) RenderRenderDocPanel(&state.showRenderDocPanel);
        if (state.showGpuDebuggerPanel) RenderGpuDebuggerPanel(&state.showGpuDebuggerPanel);
        if (state.showMemoryProfilerPanel) RenderMemoryProfilerPanel(&state.showMemoryProfilerPanel);
        if (state.showPackageManagerPanel) RenderPackageManagerPanel(&state.showPackageManagerPanel);
        if (state.showPluginManagerPanel) RenderPluginManagerPanel(&state.showPluginManagerPanel);
        if (state.showLocalizationPanel) RenderLocalizationPanel(&state.showLocalizationPanel);
        if (state.showConsoleVariablesPanel) RenderConsoleVariablesPanel(&state.showConsoleVariablesPanel);
        if (state.showAssetRegistryPanel) RenderAssetRegistryPanel(&state.showAssetRegistryPanel);
        if (state.showNavigationBuilderPanel) RenderNavigationBuilderPanel(&state.showNavigationBuilderPanel);
        if (state.showLightBakingPanel) RenderLightBakingPanel(&state.showLightBakingPanel);

        // Context-Sensitive & Workflow Editors
        if (state.showMaterialEditorPanel) RenderMaterialEditorPanel(&state.showMaterialEditorPanel);
        if (state.showBlueprintEditorPanel) RenderBlueprintEditorPanel(&state.showBlueprintEditorPanel);
        if (state.showAnimationWorkspacePanel) RenderAnimationWorkspacePanel(&state.showAnimationWorkspacePanel);
        if (state.showAudioEditorPanel) RenderAudioEditorPanel(&state.showAudioEditorPanel);
        if (state.showSequencerPanel) RenderSequencerPanel(&state.showSequencerPanel);
    }
    else if (mode == WorkspaceMode::Codebase) {
        RenderProjectExplorerPanel();
        RenderCodeEditorPanel();
        RenderCodeSymbolsPanel();
        RenderCodeBottomDockPanel();
    }
    else if (mode == WorkspaceMode::Run) {
        RenderRunViewportPanel();
        RenderRunBottomDockPanel();
    }

    // Render Modals
    RenderImportProgressModal();
    RenderProjectWizardModal(&state.showProjectWizardModal);
    RenderProjectSettingsModal(&state.showProjectSettingsModal);

    if (state.showAboutModal) {
        ImGui::OpenPopup("About Blueman Engine");
        if (ImGui::BeginPopupModal("About Blueman Engine", &state.showAboutModal, ImGuiWindowFlags_AlwaysAutoResize)) {
            const auto& pal = Theme::GetPalette();
            ImGui::TextColored(pal.accent, "Blueman Engine v3.5 Enterprise Edition");
            ImGui::TextColored(pal.textSecondary, "Next-Generation DirectX 12 High-Performance Rendering Suite");
            ImGui::Separator();
            ImGui::Spacing();
            ImGui::BulletText("Renderer: DirectX 12 Agility SDK (DXR 1.1 / Mesh Shaders)");
            ImGui::BulletText("Asset Pipeline: Background Asset Cooker & Virtual File System");
            ImGui::BulletText("Scripting Architecture: Zelyn Language Compiler Integration");
            ImGui::BulletText("Engine Architecture: Lock-Free Multithreaded Subsystem Graph");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            if (ImGui::Button("Close", ImVec2(120.0f, 26.0f))) {
                state.showAboutModal = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    // 7. Render Fixed Bottom Status Bar
    RenderStatusBar();
}

void ShutdownApplication() {
    // Explicitly shut down subsystems in reverse initialization order.
    // Critical: BackgroundAssetCooker worker thread must be stopped
    // before static destructors run, to prevent use-after-free.
    Logger::Get().Info("[Application] Shutting down editor subsystems...");
    BackgroundAssetCooker::Get().~BackgroundAssetCooker();
    CommandStack::Get().Clear();
    Logger::Get().Info("[Application] Shutdown complete.");
}

} // namespace EngineEditor

