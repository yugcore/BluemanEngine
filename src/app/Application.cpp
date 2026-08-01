#include "Application.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"
#include "core/EditorState.h"

#include "panels/Chrome/CustomTitleBar.h"
#include "panels/Chrome/MenuBar.h"
#include "panels/Chrome/WorkspaceBar.h"
#include "panels/Chrome/Toolbar.h"
#include "panels/Chrome/StatusBar.h"

// Editor Panels
#include "panels/Viewport/ViewportPanel.h"
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

namespace EngineEditor {

void InitializeApplication() {
}

void RenderApplicationLayout() {
    auto& state = EditorState::Get();

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
        if (state.showObjectPalettePanel) RenderObjectPalettePanel(&state.showObjectPalettePanel);
        RenderContentBrowserPanel();
        RenderViewportPanel();
        RenderOutlinerPanel();
        RenderDetailsPanel();
        RenderOutputLogPanel();
        RenderRenderControlStripPanel();
        if (state.showMeshStudioPanel) RenderMeshStudioPanel(&state.showMeshStudioPanel);
        if (state.showShaderStudioPanel) RenderShaderStudioPanel(&state.showShaderStudioPanel);
        if (state.showTextureViewerPanel) RenderTextureViewerPanel(&state.showTextureViewerPanel);
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
    RenderProjectWizardModal(&state.showProjectWizardModal);
    RenderProjectSettingsModal(&state.showProjectSettingsModal);

    // 7. Render Fixed Bottom Status Bar
    RenderStatusBar();
}

void ShutdownApplication() {
}

} // namespace EngineEditor
