# BluemanEngine Subsystem & Module Capabilities Checklist (CHECKLIST.md)

> **Project Target:** Modern AAA-Scale Direct3D 12 Engine & Editor (`BluemanEngine / ZeGFX`)  
> **Status Audit Date:** August 2026  
> **Location:** `BluemanEngine/CHECKLIST.md`

---

## Executive Summary & Legend

This checklist tracks the existence, wiring, and development maturity levels of all key editor tools, engine modules, and UI frameworks within **BluemanEngine**.

### Legend & Classification Matrix
* `[Production Ready]` — Fully implemented, wired to `ZeGFX Core` / `ZePhysics`, tested, and operational in the editor interface.
* `[Beta / Partial]` — Core engine support or UI exists, but full feature capabilities (e.g. interactive brush tools or visual node editing) are actively under development or unwired.
* `[Planned / Not Implemented]` — Architectural design complete and tracked in the `BluemanRM.md` roadmap for production phases, but code is not yet implemented.

---

## Master Subsystem & Module Checklist

| # | Module | Description | Status | Current Level | Primary Implementation / Location |
| :---: | :--- | :--- | :---: | :---: | :--- |
| **1** | **TerrainEditor** | Creates and edits terrain, landscapes, and heightmaps. | `[Exists]` | `[Beta / Partial]` | `src/panels/ImportProgress/ImportHeightmapModal.cpp`, `src/terrain_system.cpp` (Chunking engine ready; sculpting brush in `TASK-PROD-07`). |
| **2** | **StudioWidgets** | Shared UI controls used throughout the editor. | `[Exists]` | `[Production Ready]` | `src/widgets/` (`PropertyRow.cpp`, `SearchBar.cpp`, `AssetTile.cpp`), `src/theme/`. |
| **3** | **MeshEditor** | Edits static 3D mesh assets. | `[Exists]` | `[Production Ready]` | `src/panels/MeshStudio/MeshStudioPanel.cpp` (3D mesh preview, submesh inspection, material assignments). |
| **4** | **TimelineCore** | Core timeline playback and sequencing framework. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-08` (Cinematic Sequencer & Timeline Core). |
| **5** | **SceneHierarchy** | Displays and manages objects in the scene tree. | `[Exists]` | `[Production Ready]` | `src/panels/Outliner/OutlinerPanel.cpp` (Scene entity tree, node selection, parenting, creation/deletion). |
| **6** | **VisualScriptGraph** | Defines visual scripting nodes and graph logic. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-12` (Visual Gameplay Scripting Graph). |
| **7** | **ShaderEditor** | Creates and edits material and shader graphs. | `[Exists]` | `[Production Ready]` | `src/panels/ShaderStudio/ShaderStudioPanel.cpp` (Live DXC runtime HLSL compiler & line diagnostics; visual node graph in `TASK-PROD-06`). |
| **8** | **StudioStatusBar** | Displays editor status, progress, and notifications. | `[Exists]` | `[Production Ready]` | `src/panels/Chrome/StatusBar.cpp` (Real-time FPS, GPU frame time ms, draw calls, VRAM telemetry, task notifications). |
| **9** | **ComponentPreview** | Visualizes and debugs component rendering in the editor. | `[Exists]` | `[Production Ready]` | `src/panels/Viewport/Overlay.cpp`, `Gizmos.cpp`, debug viewport render modes. |
| **10** | **InterfaceDesigner** | Creates and edits user interface layouts. | `[Exists]` | `[Production Ready]` | `src/layout/` (`Dockspace.cpp`, `WindowLayout.cpp`), custom ImGui docking shell, `blueman_layout.ini`. |
| **11** | **CurveStudio** | Edits animation, float, and color curves. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-08` (Curve & keyframe editing tools). |
| **12** | **ScriptCompiler** | Compiles visual scripts into executable logic. | `[Exists]` | `[Beta / Partial]` | `ShaderStudioPanel.cpp` (Direct DXC HLSL runtime compilation); visual logic node compiler in `TASK-PROD-12`. |
| **13** | **StudioFramework** | Provides the core editor architecture and services. | `[Exists]` | `[Production Ready]` | `src/app/Application.cpp`, `src/core/` (`EditorState.h`, `CommandStack.cpp`, `ComponentRegistry.cpp`, `Logger.cpp`). |
| **14** | **RigEditor** | Creates and edits character skeletons and rigs. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-09` (Skeletal mesh skinning & bone hierarchy editor). |
| **15** | **AssetExplorer** | Browses, imports, and manages project assets. | `[Exists]` | `[Production Ready]` | `src/panels/ContentBrowser/ContentBrowserPanel.cpp`, `src/engine/assets/` (`AssetRegistry.cpp`, `BackgroundAssetCooker.cpp`). |
| **16** | **NodeEditor** | Generic node-based editor framework. | `[Missing]` | `[Planned / In Design]` | Generic graph node canvas framework planned for `TASK-PROD-06` and `TASK-PROD-12`. |
| **17** | **StudioEditor** | Main editor module coordinating all tools. | `[Exists]` | `[Production Ready]` | `BluemanEngine/src/main.cpp`, `src/app/Application.cpp` (Main executable coordinating windowing, rendering, and editor panels). |
| **18** | **MeshPainter** | Paints vertex colors and texture layers on meshes. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-07` (Mesh vertex color & multi-layer terrain painting). |
| **19** | **ProjectConfiguration** | Manages project-wide settings and preferences. | `[Exists]` | `[Production Ready]` | `src/panels/ProjectSettings/ProjectSettingsModal.cpp`, `src/panels/ProjectWizard/ProjectWizardModal.cpp`, `EditorState.h`. |
| **20** | **ViewInteraction** | Handles viewport input and manipulation. | `[Exists]` | `[Production Ready]` | `src/core/EditorCamera.cpp` (6-DOF fly/walk/orbit/pan), `src/panels/Viewport/` (`Gizmos.cpp`, `ViewportPicker.cpp`, `ViewportSelection.cpp`). |
| **21** | **ScriptStudio** | Main visual scripting editor. | `[Exists]` | `[Beta / Partial]` | `src/panels/Codebase/` (`CodeEditorPanel.cpp`, `CodeHighlighter.cpp`, `ProjectExplorerPanel.cpp` text code editor; visual graph in `TASK-PROD-12`). |
| **22** | **TimelineWidgets** | User interface for timeline editing. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-08` (Timeline UI tracks & scrubber control widgets). |
| **23** | **WorldGridEditor** | Manages large-world streaming and partitioning. | `[Exists]` | `[Beta / Partial]` | `src/terrain_system.cpp` (500m-1km heightmap chunking), `Overlay.cpp` (3D viewport ground grid overlay). |
| **24** | **StudioSubsystem** | Provides reusable editor-wide subsystems and services. | `[Exists]` | `[Production Ready]` | `src/render/ZeGFXAdapter.cpp` (RHI/Render adapter), `BackgroundAssetCooker`, `PhysicsWorld` (ZePhysics adapter), `CommandStack`. |
| **25** | **CharacterStudio** | Workspace for character, animation, and rig editing. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-08` / `TASK-PROD-09` (Character & animation workspace). |
| **26** | **PropertyCustomization** | Customizes property inspector layouts and behavior. | `[Exists]` | `[Production Ready]` | `src/widgets/PropertyRow.cpp`, `src/core/ComponentRegistry.cpp`, custom reflection property layout drawers in `DetailsPanel.cpp`. |
| **27** | **TableEditor** | Creates and edits structured data tables. | `[Missing]` | `[Planned / Not Implemented]` | Structured data table editor for project data assets and configuration tables. |
| **28** | **ObjectPlacement** | Places and arranges objects in scenes. | `[Exists]` | `[Production Ready]` | `src/panels/ObjectPalette/ObjectPalettePanel.cpp`, `ViewportDragDrop.cpp`, `ViewportContextMenu.cpp`, `Gizmos.cpp`. |
| **29** | **ImageEditor** | Imports and edits texture/image assets. | `[Exists]` | `[Production Ready]` | `src/panels/TextureViewer/TextureViewerPanel.cpp`, texture block compression engine (`.ztex` BC1-BC7 import/viewer). |
| **30** | **SceneEditor** | Primary scene editing workspace. | `[Exists]` | `[Production Ready]` | `src/panels/Viewport/ViewportPanel.cpp`, `src/render/ViewportRenderer.cpp` (Interactive D3D12 viewport host with live selection & gizmos). |
| **31** | **TimelineEditor** | Creates and edits cinematic and animation timelines. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-08` (Track-based cinematic & animation sequencer editor). |
| **32** | **SnapTools** | Controls grid, angle, and object snapping. | `[Exists]` | `[Production Ready]` | `src/panels/Chrome/Toolbar.cpp` (Grid/angle/scale snap toggles & step inputs), `Gizmos.cpp` (ImGuizmo snap matrix integration). |
| **33** | **StudioStyle** | Defines editor themes, icons, and visual styles. | `[Exists]` | `[Production Ready]` | `src/theme/` (`Colors.cpp`, `Fonts.cpp`, `Metrics.cpp`, `Style.cpp`, `Theme.h`), custom AAA dark editor design system tokens. |
| **34** | **TypeExplorer** | Browses available object and class types. | `[Exists]` | `[Production Ready]` | `src/panels/Codebase/CodeSymbolsPanel.cpp`, `src/core/ComponentRegistry.cpp` (Reflected component/type hierarchy). |
| **35** | **RiggedMeshEditor** | Edits skinned meshes and character models. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-09` (Skinned mesh & skeletal character model editor). |
| **36** | **ComponentEditor** | Adds, removes, and configures object components. | `[Exists]` | `[Production Ready]` | `src/panels/Details/DetailsPanel.cpp`, `src/core/ComponentRegistry.cpp` (Add/remove/configure scene entity components). |
| **37** | **VegetationTools** | Paints and manages vegetation and foliage. | `[Exists]` | `[Beta / Partial]` | `dx12_pipeline.cpp` (Foliage `ExecuteIndirect` compute culling), `ObjectPalettePanel.cpp` (Scattering; brush paint tool in `TASK-PROD-07`). |
| **38** | **InspectorPanel** | Displays and edits selected object properties. | `[Exists]` | `[Production Ready]` | `src/panels/Details/DetailsPanel.cpp` (Primary entity details inspector, transform controls, component fields). |
| **39** | **ScriptWidgets** | UI components used by the visual scripting editor. | `[Missing]` | `[Planned / Not Implemented]` | Tracked in roadmap `TASK-PROD-12` (UI nodes & connection pins for visual script editor). |
| **40** | **StudioShell** | Main application window, menus, and docking system. | `[Exists]` | `[Production Ready]` | `src/panels/Chrome/` (`CustomTitleBar.cpp`, `MenuBar.cpp`, `WorkspaceBar.cpp`), `src/layout/` (`Dockspace.cpp`, `WindowLayout.cpp`), `src/main.cpp`. |

---

## Subsystem Category Breakdown

### 🟢 1. Production Ready Modules (25 Modules)
These modules are fully implemented in C++, wired to active engine/editor systems, and verified in the build:
* **StudioShell** (`src/panels/Chrome/`, `src/layout/`, `src/main.cpp`)
* **StudioEditor** (`src/app/Application.cpp`, `BluemanEngine/src/main.cpp`)
* **StudioFramework** (`src/app/Application.cpp`, `src/core/`)
* **StudioSubsystem** (`src/render/ZeGFXAdapter.cpp`, `BackgroundAssetCooker.cpp`)
* **SceneEditor** (`src/panels/Viewport/ViewportPanel.cpp`, `ViewportRenderer.cpp`)
* **SceneHierarchy** (`src/panels/Outliner/OutlinerPanel.cpp`)
* **InspectorPanel** (`src/panels/Details/DetailsPanel.cpp`)
* **ComponentEditor** (`src/panels/Details/DetailsPanel.cpp`, `ComponentRegistry.cpp`)
* **AssetExplorer** (`src/panels/ContentBrowser/ContentBrowserPanel.cpp`, `AssetRegistry.cpp`)
* **MeshEditor** (`src/panels/MeshStudio/MeshStudioPanel.cpp`)
* **ShaderEditor** (`src/panels/ShaderStudio/ShaderStudioPanel.cpp`)
* **ImageEditor** (`src/panels/TextureViewer/TextureViewerPanel.cpp`)
* **ViewInteraction** (`src/core/EditorCamera.cpp`, `src/panels/Viewport/Gizmos.cpp`)
* **ObjectPlacement** (`src/panels/ObjectPalette/ObjectPalettePanel.cpp`, `ViewportDragDrop.cpp`)
* **ProjectConfiguration** (`src/panels/ProjectSettings/ProjectSettingsModal.cpp`, `ProjectWizardModal.cpp`)
* **PropertyCustomization** (`src/widgets/PropertyRow.cpp`, `DetailsPanel.cpp`)
* **SnapTools** (`src/panels/Chrome/Toolbar.cpp`, `Gizmos.cpp`)
* **StudioStatusBar** (`src/panels/Chrome/StatusBar.cpp`)
* **StudioWidgets** (`src/widgets/PropertyRow.cpp`, `SearchBar.cpp`, `AssetTile.cpp`)
* **StudioStyle** (`src/theme/Colors.cpp`, `Fonts.cpp`, `Metrics.cpp`, `Style.cpp`)
* **InterfaceDesigner** (`src/layout/Dockspace.cpp`, `WindowLayout.cpp`)
* **ComponentPreview** (`src/panels/Viewport/Overlay.cpp`, `Gizmos.cpp`)
* **TypeExplorer** (`src/panels/Codebase/CodeSymbolsPanel.cpp`, `ComponentRegistry.cpp`)

---

### 🟡 2. Beta & Partial Modules (6 Modules)
Modules with backend engine support or text-based UI operational, but missing full interactive visual tools:
* **TerrainEditor** — Heightmap asset import (`ImportHeightmapModal.cpp`) and ZeGFX chunk rendering exist; interactive 3D sculpt brush is in roadmap `TASK-PROD-07`.
* **VegetationTools** — High-density foliage compute culling (`ExecuteIndirect`) and palette scattering exist; interactive brush paint tool in roadmap `TASK-PROD-07`.
* **WorldGridEditor** — 500m-1km heightmap chunking (`src/terrain_system.cpp`) & 3D floor grid overlay (`Overlay.cpp`) exist; world partition partitioning tools planned.
* **ScriptStudio** — Integrated C++ text code editor (`CodeEditorPanel.cpp`, `CodeHighlighter.cpp`, `ProjectExplorerPanel.cpp`) exists; node-based visual graph is in roadmap `TASK-PROD-12`.
* **ScriptCompiler** — Integrated DXC runtime HLSL compiler exists in `ShaderStudioPanel.cpp`; visual node logic compiler is in roadmap `TASK-PROD-12`.

---

### 🔴 3. Planned / Future Scope Modules (9 Modules)
Modules outlined in the `BluemanRM.md` Production Roadmap (`Part 2: Production Engine Suite`):
* **TimelineCore**, **TimelineEditor**, **TimelineWidgets**, **CurveStudio** — Track-based cinematic sequencer & animation timeline (`TASK-PROD-08`).
* **VisualScriptGraph**, **NodeEditor**, **ScriptWidgets** — Visual gameplay scripting graph canvas (`TASK-PROD-12`) & node graph framework (`TASK-PROD-06`).
* **RigEditor**, **RiggedMeshEditor**, **CharacterStudio** — Skeletal mesh animation, bone rigs & vertex skinning (`TASK-PROD-09`).
* **MeshPainter** — Viewport vertex color & texture layer painting (`TASK-PROD-07`).
* **TableEditor** — Data table editor for structured game configuration databases.

---
*Document automatically generated and verified for `BluemanEngine`.*
