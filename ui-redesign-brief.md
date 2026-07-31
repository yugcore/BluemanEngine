# ZeGFX Editor — UI/UX Redesign Brief

## Goal
Redesign the ZeGFX (Blueman Engine) editor interface to match the visual polish, hierarchy, and confidence of Unreal Engine 5's editor — without copying it wholesale. The current UI is functional but reads as an unstyled debug shell: one font, one accent color, no icons, no depth, no custom chrome.

---

## 1. Typography
- Use a single clean sans-serif UI font family (e.g. Inter, Segoe UI Variable, or a Roboto-derivative) for all UI chrome — reserve monospace *only* for the console/log output and numeric code-like fields.
- Define a real type scale, minimum 4 tiers:
  - Panel/window titles: 13–14px, semibold
  - Section headers (e.g. "Hardware Ray Tracing (DXR)"): 12px, semibold, slightly muted color
  - Field labels / body text: 11–12px, regular
  - Secondary/meta text (status bar, tooltips): 10–11px, regular, lower-contrast gray
- Remove all redundant bracket-label buttons (`[Save] Save` → just `Save`, ideally with an icon and no text at all for common actions).

## 2. Color system
Define a token-based palette, not ad hoc grays:
- `bg-base` (main window): #0F0F11 or similar near-black
- `bg-panel`: one step lighter than base (#17171A)
- `bg-panel-header`: one step lighter again (#1E1E22)
- `bg-elevated` (floating panels, popovers): #232327 + soft drop shadow (0 4px 12px rgba(0,0,0,0.4))
- `border-subtle`: low-contrast 1px hairlines (#2A2A2E), used sparingly — prefer elevation/shadow over borders for separation
- `accent-primary` (one color only, for primary actions + active states): a single blue, e.g. #4C8EFF
- `accent-selection` (viewport/outliner selection): a distinct color from primary accent, e.g. amber/orange (#F2A93B) — Unreal-style selection outline
- Semantic status colors, defined once and reused consistently: success/active green, warning yellow, error red, info blue — never repurposed for unrelated UI
- Text: primary #E8E8EA, secondary #9A9AA2, disabled #55555C

## 3. Iconography
- Every toolbar action (Save, Undo, Redo, Import, Select, Move, Rotate, Scale, Play, Stop) gets a 16–20px icon, text becomes a tooltip on hover, not inline label.
- Content Browser items get real thumbnail previews (or a type-specific icon: mesh, material, texture) instead of colored dots.
- Panel tabs (Content Browser, Viewport, Outliner, Details, Output Log) get a small leading icon.
- Use a single consistent icon set/style (either all outline or all filled — never mixed) at a fixed stroke weight.

## 4. Layout, spacing & elevation
- Adopt an 8px base spacing grid for all padding/margins; property rows need consistent vertical rhythm (currently cramped).
- Replace the floating "Render Control Strip" overlay with a proper dockable panel (tabbed alongside Details/Outliner, or a slide-out drawer) — nothing should float loose on top of the 3D viewport.
- Viewport HUD stats (GPU name, VRAM, triangle count) become a translucent, borderless overlay (background rgba(0,0,0,0.35), blurred if possible) anchored to a corner — not an opaque bordered box competing with the render.
- Round panel/card corners slightly (4–6px radius) and use soft shadows instead of hard borders to separate elevated surfaces (dropdowns, floating panels, modals).
- Outliner rows: add subtle alternating row tint or hover-state highlight, and align columns (Name / World / Panel) to a real grid with consistent right-padding.

## 5. Chrome & structural polish
- Custom titlebar matching the dark theme (including custom minimize/maximize/close glyphs) — no default OS white titlebar.
- Selection gizmo: replace the plain rectangle outline with corner-handle brackets + colored axis gizmo (red/green/blue XYZ), consistent with the accent-selection color for the outline itself.
- Status bar: group related stats visually (icon + value clusters with small gaps) instead of one long pipe-delimited string; keep it low-contrast/secondary text since it's ambient info, not primary content.
- Buttons get real states: default / hover (subtle lighten) / active-pressed (accent fill) / disabled (reduced opacity) — currently every button looks identical regardless of state.

## 6. Reference direction
Target the visual language of: Unreal Engine 5 editor, Unity's Pro dark theme, Blender 4.x, and Figma's dark mode — flat-but-elevated dark UI, single restrained accent color, icon-driven toolbars, generous but efficient spacing, no visual noise from redundant text or inconsistent borders.
