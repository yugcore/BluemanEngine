#include "Application.h"
#include "layout/Dockspace.h"
#include "layout/WindowLayout.h"

#include "panels/Chrome/CustomTitleBar.h"
#include "panels/Chrome/MenuBar.h"
#include "panels/Chrome/Toolbar.h"
#include "panels/Chrome/StatusBar.h"

#include "panels/Viewport/ViewportPanel.h"
#include "panels/ContentBrowser/ContentBrowserPanel.h"
#include "panels/Outliner/OutlinerPanel.h"
#include "panels/Details/DetailsPanel.h"
#include "panels/OutputLog/OutputLogPanel.h"
#include "panels/RenderControlStrip/RenderControlStripPanel.h"

namespace EngineEditor {

void InitializeApplication() {
}

void RenderApplicationLayout() {
    // 1. Render unified title bar (includes menu bar inline)
    RenderCustomTitleBar();
    
    // 2. Render standalone toolbar below
    RenderToolbar();

    // 3. Calculate Dockspace Host Bounds
    float topOffset = GetTitleBarTotalHeight() + GetToolbarTotalHeight();
    Layout::DockspaceBounds bounds = Layout::CalculateDockspaceBounds(topOffset, GetStatusBarTotalHeight());
    ImGuiID dockspaceId = Layout::RenderDockspaceHost(bounds);

    // 4. Setup Default Docking Layout on first run
    Layout::SetupDefaultLayout(dockspaceId, bounds);

    // 5. Render Primary Engine Panel Windows
    RenderContentBrowserPanel();
    RenderViewportPanel();
    RenderOutlinerPanel();
    RenderDetailsPanel();
    RenderOutputLogPanel();

    // 6. Render Floating Controls & Fixed Bottom Status Bar
    RenderRenderControlStripPanel();
    RenderStatusBar();
}

void ShutdownApplication() {
}

} // namespace EngineEditor
