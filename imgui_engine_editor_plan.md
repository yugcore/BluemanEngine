# Dear ImGui Engine-Editor — Phased Build Plan

Recreating the "BLUEMAN ENGINE v2.0 Enterprise" style layout: main menu + toolbar, dockable Content Browser / Viewport / Outliner / Details / Output Log, a floating Render Control Strip, and a bottom status bar.

## Assumptions (change Phase 0 if these are wrong)

- **Language:** C++17
- **GUI lib:** [Dear ImGui, `docking` branch](https://github.com/ocornil/imgui) (you need docking + multi-window support — this is not in `master`)
- **Windowing/render backend:** GLFW + OpenGL3 (fastest to stand up, cross-platform). If you're actually integrating into a real engine with DX12 (the screenshot says "DR12"), swap the backend files in Phase 0 only — every later phase is backend-agnostic since it's all ImGui calls.
- **Build system:** CMake + FetchContent (no manual vendoring)
- **Extra libs:** ImGuizmo (3D manipulate gizmo + view cube), IconFontCppHeaders + Font Awesome or Material Design Icons (toolbar/tree icons), ImGuiFileDialog (Import button)

If any of this doesn't match your project, tell me and I'll rewrite Phase 0 — everything after it is written to be stack-agnostic.

## Suggested project structure

```
engine_editor/
  CMakeLists.txt
  third_party/            (FetchContent-managed)
  src/
    main.cpp
    app/
      Application.h/.cpp       # window, ImGui init, main loop, dockspace
      Theme.h/.cpp             # colors, rounding, fonts, icon font merge
    core/
      SceneGraph.h/.cpp        # scene node tree, actor/component data
      Selection.h/.cpp         # shared selection state
      CommandStack.h/.cpp      # undo/redo
      AssetRegistry.h/.cpp     # content browser backing data
      EditorState.h            # single struct all panels read/write
    panels/
      MenuBar.h/.cpp
      Toolbar.h/.cpp
      ContentBrowserPanel.h/.cpp
      ViewportPanel.h/.cpp
      RenderControlStripPanel.h/.cpp
      OutlinerPanel.h/.cpp
      DetailsPanel.h/.cpp
      OutputLogPanel.h/.cpp
      StatusBar.h/.cpp
    render/
      ViewportRenderer.h/.cpp  # offscreen FBO the viewport image comes from
  assets/fonts/
```

## How to use this doc

Paste each **Agent Prompt** block into your IDE agent in order, one phase per session. Build and eyeball the result before moving on — later phases assume earlier panels exist and compile. Each prompt is self-contained (recaps context) so you don't have to re-explain the project each time.

---

## Phase 0 — Project scaffolding & window shell

**Goal:** empty window with ImGui docking enabled, nothing else.

**Agent Prompt:**
```
Set up a new C++17 CMake project called "engine_editor". Use FetchContent to pull in:
- Dear ImGui (docking branch, github.com/ocornut/imgui)
- GLFW
- glad (OpenGL 3.3 core loader)

Create src/main.cpp that:
1. Initializes GLFW + an OpenGL 3.3 context, window title "BLUEMAN ENGINE v2.0 Enterprise", 1920x1080, resizable.
2. Initializes ImGui with ImGuiConfigFlags_DockingEnable and ImGuiConfigFlags_ViewportsEnable.
3. Runs a main loop that starts an ImGui frame, calls ImGui::DockSpaceOverViewport() on a fullscreen host window, shows ImGui::ShowDemoWindow() for now, and renders.
4. Cleans up on exit.

Set up the folder structure:
src/app, src/core, src/panels, src/render, assets/fonts.

Confirm it builds and a window opens with docking working (I should be able to drag the demo window and dock it).
```

**Definition of done:** window opens, ImGui demo window can be docked/undocked, no crashes on close.

---

## Phase 1 — App shell: menu bar, toolbar, dockspace layout, dark theme

**Goal:** the chrome from the top of the screenshot — menu bar, icon toolbar row, and a default dock layout with empty placeholder panels in the right spots.

**Agent Prompt:**
```
Continuing the engine_editor project from Phase 0. Build the application shell:

1. src/app/Theme.cpp: a dark theme close to Unreal/Unity editor style — dark navy-gray backgrounds (#1e1e22 range), a blue accent color (#2f8fd4 range) for active/selected states, 4px rounding on buttons and frames, compact frame padding. Apply it once at startup.

2. src/panels/MenuBar.cpp: a top menu bar with File, Edit, Create, Window, Build, Help. Each is a real ImGui menu with placeholder MenuItems (e.g. File > New, Open, Save, Exit) — just print to console on click for now. Show the app name/logo text and a workspace name ("ZeGFX Workspace") right-aligned in the menu bar, matching a title-bar style strip.

3. src/panels/Toolbar.cpp: a horizontal icon+label button strip directly under the menu bar: Save | Undo | Redo | Import ▾ | Add ▾ | Select | Move | Rotate | Scale | Snap-toggle-with-value(0.50) | Play ▾ | Frame | Stop | Build ▾, then right-aligned: Perspective ▾, a couple of view-mode icon buttons, a settings gear icon. Use plain ImGui::Button for now with text labels (icons come in Phase 10) — focus on layout, grouping, and consistent spacing/height.

4. src/app/Application.cpp: replace the demo window with a fixed default dock layout built via the ImGui docking API (DockBuilder): left dock = "Content Browser", center = "Viewport", right dock split top/bottom = "Outliner" / "Details", bottom dock = "Output Log". For now each is just an empty ImGui::Begin/End window with a placeholder text label so the layout is visible. Build this layout only once (on first run / if no imgui.ini exists).

Confirm: menu bar and toolbar render correctly, and the 5 panels dock into the Unreal-style layout (left/center/right/bottom) shown in reference screenshots of professional game engine editors.
```

**Definition of done:** layout visually matches the reference regions (left/center/right-split/bottom), theme reads as dark/professional, toolbar buttons are clickable placeholders.

---

## Phase 2 — Content Browser panel

**Agent Prompt:**
```
Continuing engine_editor. Implement src/panels/ContentBrowserPanel.cpp, replacing the placeholder "Content Browser" window.

Backing data: src/core/AssetRegistry.h/.cpp — a simple in-memory tree (AssetFolder { name, children folders, vector<AssetItem> }), seeded with dummy data matching this shape:
  ZeGFX Workspace
    Blueman Cooked Assets
      Meshes: M_Village_Wall_01_Base, M_Village_Gate_Detailed, M_Village_Ground_Terrain_H
      Materials: Mat_Village_Architecture_PBR, Mat_Village_Terrain_Auto, Mat_Sky_Atmosphere_Advanced
      Textures: T_Village_Wall_Albedo, T_Village_Wall_Normal, T_Village_Wall_Roughness
      Blueprints: BP_GameController, BP_Npc_Villager, BP_Npc_Villager_Havager
      AI: BT_Villager_Behaviors
      Level_Scripts: LS_Atmospheric_Sequence

Panel UI:
1. Top row: "+ Add" / "Import" / "Save All" buttons.
2. A search text input that filters the tree below by substring match (case-insensitive), across folder and item names.
3. A two-pane layout: left is a collapsible tree (ImGui::TreeNode per folder, indent per depth, folder icon placeholder via "[+]"/"[-]" prefix), right/below shows the selected folder's items as a simple list (thumbnail placeholder square + name), each item type gets a distinct color tag (mesh/material/texture/blueprint/etc).
4. Clicking an item sets a global "selected asset" (add this to src/core/EditorState.h as a simple struct all panels can read — introduce EditorState now, it will grow in later phases).

Confirm: tree expands/collapses, search filters live, selecting an item is visibly highlighted.
```

---

## Phase 3 — Viewport panel + stats overlay

**Agent Prompt:**
```
Continuing engine_editor. Implement the Viewport panel, replacing the placeholder window.

1. src/render/ViewportRenderer.cpp: create an offscreen framebuffer (FBO + color texture) sized to the viewport panel's content region, resized each frame if the panel is resized. For now render a simple rotating colored triangle or a textured quad into it via basic OpenGL calls — this is a stand-in for real scene rendering.

2. src/panels/ViewportPanel.cpp:
   - Render the FBO texture into the panel with ImGui::Image, filling available content region.
   - Top toolbar row inside the viewport (an ImGui child region or overlaid buttons): "Perspective ▾" dropdown, a numeric/quality dropdown, "Show ▾" dropdown, 3 small gizmo-mode icon buttons, then right-aligned status pills: renderer tag (e.g. "DR12"), "FPS: xx.x", "Frame: x.xx ms", a partition/DRR tag, "Entities: N ▾". Style these as small rounded rect "pill" buttons, not plain buttons.
   - A semi-transparent stats overlay box top-left (use ImGui::SetNextWindowBgAlpha + no-decoration child window) listing: GPU name, VRAM usage, Triangle count, Draw calls, DLSS/upscaler mode, RTX GI status, Volumetric Lighting status, Nanite status — pull these from a new EditorState.RenderStats struct, hardcode plausible values for now.
   - A small 3D orientation gizmo cube in the top-right corner of the viewport (a simple ImGui-drawn cube using ImDrawList with 3 visible faces is fine for now; note that ImGuizmo's ViewManipulate can replace this later).

Confirm: viewport shows live rendered content, resizes correctly with the panel, overlay and toolbar don't block mouse interaction with the 3D content underneath except where they're actually drawn.
```

---

## Phase 4 — Floating Render Control Strip

**Agent Prompt:**
```
Continuing engine_editor. Add a new floating (non-docked, closable) panel: src/panels/RenderControlStripPanel.cpp, titled "Render Control Strip", opened via a toolbar/menu toggle and positioned initially overlapping the top-left of the viewport.

Contents, top to bottom, using ImGui::CollapsingHeader for each section (default open):

1. Info box (styled child region, slightly different background): GPU name, "VRAM Usage: x.x GB / y GB", a highlighted status line for current graphics API / ray tracing mode.
2. "Global Quality Presets:" label + a row of buttons: Low, Medium, High, Save — mutually-highlight the active one.
3. Header "Hardware Ray Tracing (DXR)":
   - 3 checkboxes: "Ray Traced Global Illumination (RTGI) [Quality: Ultra, Bounces: 4]", "Ray Traced Ambient Occlusion (RTAO)", "Ray Traced Reflections (RTR)"
   - a slider (0.5x–2.0x) labeled with its current value e.g. "1.00x", plus a "DSR Resolution Scale" checkbox next to it
4. Header "World Partition & Spatial Grid Streaming":
   - slider "Cell Size (m)" (e.g. 50–500, default 150)
   - slider "Streaming Radius (m)" (e.g. 100–1000, default 600)
   - greyed-out info text below: "Active Streamed Level: N | Total Spatial Scale: N"
5. Header "Nanite Virtual Geometry & Mesh Shader Pipeline": 3 checkboxes for cluster/culling/mesh-shader toggles, greyed info text below with counts.
6. Header "Quick Isolation Tests (MRQ Floating)": a checklist (8ish items) — geometry/texture/lighting/shadows/DXR/material/mesh/render isolation toggles, all default unchecked.

All values live in a new EditorState.RenderSettings struct so other panels can read them later. Panel has a close (X) button that just hides it (toggle from toolbar re-shows it).

Confirm: panel floats freely over the viewport, all headers collapse independently, sliders/checkboxes retain state.
```

---

## Phase 5 — Outliner panel

**Agent Prompt:**
```
Continuing engine_editor. Implement src/panels/OutlinerPanel.cpp and src/core/SceneGraph.h/.cpp, replacing the placeholder "Outliner" window.

SceneGraph: a tree of SceneNode { name, type (Folder/Actor/Light/Camera/Audio/Component), vector<SceneNode> children }. Seed dummy data:
  Environments
    Modular_Village
      Bridges (folder)
      Buildings_01 (folder)
    Sky
      SkyAtmosphere
      SunLight
      SkyLight
  Player
    CharacterActor
    CameraActor
  Audio
    AmbientSoundVolume

Panel UI:
1. Search box that filters the tree by name (case-insensitive substring), auto-expanding matching branches.
2. Tree rendered as an ImGui::Table with 3 columns: Name (with expand arrows + type icon placeholder), World, Panel — matching a typical scene-outliner column layout. Use ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg.
3. Clicking a row sets EditorState.selectedNode (shared selection — this is what the Details panel in Phase 6 will read) and highlights the row.
4. Different node types get distinct row/icon tinting (folders vs actors vs lights vs audio).

Confirm: tree matches the seed hierarchy, search/filter works, selecting a row visibly highlights and is readable back from EditorState.
```

---

## Phase 6 — Details / Properties panel

**Agent Prompt:**
```
Continuing engine_editor. Implement src/panels/DetailsPanel.cpp, replacing the placeholder "Details" window (dock it below the Outliner in the same tab well, as a separate tab).

Behavior: reads EditorState.selectedNode from the Outliner (Phase 5). If nothing is selected, show "No selection." Otherwise show:

1. Header row: selected object name + type, e.g. "SkyAtmosphere (Instance)" with an "Edit in C++" link-style button (no-op for now).
2. "Transition" section (transform block): three rows — Location, Rotation, Scale — each with a row label + dropdown-style icon + X/Y/Z numeric drag-float inputs (ImGui::DragFloat3 split into 3 individually-editable fields), plus a lock-aspect toggle icon next to Scale.
3. A component-specific section whose header matches the node type. For SkyAtmosphere specifically, implement:
   - Rayleigh Scattering Coefficient (10^-6 m^-1): drag float
   - Aerosol Scattering Scale: drag float
   - Aerosol Absorption Scale: drag float
   - Atmosphere Height: drag float with "km" suffix
   - Aerial Perspective View Distance Scale: drag float
   Below that, a small tab bar with "Ray Tracing" / "Global Illumination" tabs (content can be placeholder for now).
   For other node types (Actor, Light, Camera, Audio) show a generic placeholder "component" block so the panel doesn't crash/empty out — you'll flesh these out later, just make the pattern extensible (a map from node type -> render function).

Confirm: selecting different Outliner nodes updates this panel; SkyAtmosphere shows the full field set; transform fields are independently editable per-axis.
```

---

## Phase 7 — Output Log panel

**Agent Prompt:**
```
Continuing engine_editor. Implement src/panels/OutputLogPanel.cpp, replacing the placeholder "Output Log" window.

1. A tab bar: Console, Logs, Progress, Profiler, Output (ImGui::BeginTabBar). Console is the main functional one for now; others can be empty placeholders.
2. Console tab: a scrolling read-only text region (ImGuiTextBuffer-backed, similar to the imgui_demo Console example) that supports:
   - Color-coded lines by severity (Info=white/gray, Warning=yellow, Error=red)
   - A filter text input (only show lines containing the filter substring)
   - "Clear" button
   - Auto-scroll-to-bottom toggle (default on)
3. Wire up a global Logger (src/core/Logger.h/.cpp, simple singleton with Info/Warning/Error methods that append to the buffer with severity) and call it from Phase 2's Import/Add/Save actions and Phase 0 startup so there's real content, e.g.:
   "Engine v2.0 Enterprise Initialized."
   "Graphics Backend Loaded (OpenGL 3.3)."
   "Level 'Default_Environment' loaded (1.2s)"
   "Found N active actors"

Confirm: log lines appear on startup, filter narrows visible lines, clear empties the buffer, new actions from other panels append visibly.
```

---

## Phase 8 — Status bar

**Agent Prompt:**
```
Continuing engine_editor. Implement src/panels/StatusBar.cpp: a thin (~28px) window pinned to the bottom of the main viewport (not a dockable panel — position/size it manually each frame like a fixed HUD bar, no title bar, no resize).

Content, left to right, separated by thin vertical separator glyphs ("|"):
- colored dot + "Ready" (dot color reflects an EditorState.status enum: Ready/Building/Error)
- "FPS: xx.x"
- "Frame: x.x ms"
- "CPU: xx%"
- "VRAM: x.x / y.y GB"
- "Triangles: N" (comma-formatted)
- "Draw Calls: N"
- "Entities: N"
- "Selected: <EditorState.selectedNode name, or 'None'>"
- renderer/API string e.g. "Engine 3.5 (opengl)"
- quality preset tag e.g. "RTX Ultra" (pulls from Phase 4's active preset)
- "Editor Mode: LevelDesign"
- level name
- git branch (read via `git rev-parse --abbrev-ref HEAD` at startup, cache the result, don't shell out every frame)

Pull real values where they already exist in EditorState (selected node, quality preset); the rest can stay mocked/static for now — call out in code comments which fields are real vs mocked so they're easy to wire up later.

Confirm: bar renders as a single-line strip pinned to the bottom, doesn't overlap the Output Log dock, updates selected-object text live when Outliner selection changes.
```

---

## Phase 9 — Selection, Undo/Redo, and gizmo wiring (glue)

**Goal:** make the panels actually talk to each other and to the viewport, not just share a struct passively.

**Agent Prompt:**
```
Continuing engine_editor. Wire the panels together properly:

1. src/core/CommandStack.h/.cpp: a simple Command interface (Execute/Undo) + stack of executed commands + redo stack. Hook the toolbar's Undo/Redo buttons (Phase 1) to this. Implement one real command as proof: TransformChangeCommand, pushed whenever a Details-panel (Phase 6) transform field is edited (capture old/new value on deactivation of the drag float, not every frame).

2. Integrate ImGuizmo:
   - In the Viewport panel, when EditorState.selectedNode has a transform, draw an ImGuizmo::Manipulate gizmo over it in the correct screen position, respecting the current gizmo mode (Translate/Rotate/Scale) selected from the Phase 1 toolbar buttons.
   - Editing via the gizmo updates the same transform used by the Details panel, and goes through the same TransformChangeCommand for undo/redo consistency.
   - Replace the hand-drawn orientation cube from Phase 3 with ImGuizmo::ViewManipulate.

3. Confirm selection is fully unified: clicking an Outliner row, clicking an asset in Content Browser, and (stub) clicking an object in the viewport all funnel through one EditorState::SetSelection() call, so Details/Outliner/Viewport gizmo all stay in sync no matter which panel triggered the selection.

Confirm: dragging the gizmo in the viewport updates the Details panel numbers live; Ctrl+Z/Ctrl+Y (and the toolbar buttons) undo/redo a transform edit; selection highlight is consistent across all 3 panels.
```

---

## Phase 10 — Visual polish & icon fonts

**Agent Prompt:**
```
Continuing engine_editor. Final visual pass:

1. Merge an icon font (Font Awesome 6 solid or Material Design Icons — pick one, FetchContent it) into the ImGui font atlas at startup, using IconFontCppHeaders for the codepoint macros. Load 2 font sizes: 16px UI text, 14px for dense tables.

2. Replace text-label placeholder buttons with icon+label or icon-only versions in: the Phase 1 toolbar (save/undo/redo/select/move/rotate/scale/play/stop icons), the Phase 2 Content Browser tree (folder/mesh/material/texture/blueprint type icons), and the Phase 5 Outliner (actor/light/camera/audio type icons).

3. Tighten the theme (Phase 1): match spacing/padding/rounding more closely to the reference screenshot — compact rows in Outliner/Content Browser, slightly larger padding in the Render Control Strip sections, consistent accent-blue for all selected/active/checked states across every panel.

4. Persist panel open/closed state and the Render Control Strip's position across restarts (ImGui already does window position/size via imgui.ini — just confirm SetNextWindowPos with ImGuiCond_FirstUseEver isn't fighting it, and that ini saving is enabled).

Confirm: overall look reads as a cohesive dark professional editor, not a demo-window collage; restarting the app preserves the user's layout and open panels.
```

---

## Notes

- Each phase prompt assumes the agent can see the existing codebase — if your agent works file-by-file rather than with full repo context, paste the relevant existing file(s) alongside the prompt.
- Test/compile after every phase. It's much cheaper to catch a broken dock layout or a dangling FBO after one phase than after five.
- The seed data (asset names, scene node names, SkyAtmosphere field names) mirrors your reference image so the visual comparison stays meaningful — swap in real data whenever your actual engine backend is ready.
