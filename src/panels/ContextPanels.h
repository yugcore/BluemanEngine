#ifndef CONTEXT_PANELS_H
#define CONTEXT_PANELS_H

namespace EngineEditor {

void RenderMaterialEditorPanel(bool* pOpen);
void RenderBlueprintEditorPanel(bool* pOpen);
void RenderAnimationWorkspacePanel(bool* pOpen);
void RenderAudioEditorPanel(bool* pOpen);
void RenderProfilerPanel(bool* pOpen);
void RenderRenderDocPanel(bool* pOpen);
void RenderGpuDebuggerPanel(bool* pOpen);
void RenderSequencerPanel(bool* pOpen);
void RenderAssetRegistryPanel(bool* pOpen);
void RenderMemoryProfilerPanel(bool* pOpen);
void RenderPackageManagerPanel(bool* pOpen);
void RenderPluginManagerPanel(bool* pOpen);
void RenderLocalizationPanel(bool* pOpen);
void RenderConsoleVariablesPanel(bool* pOpen);
void RenderNavigationBuilderPanel(bool* pOpen);
void RenderLightBakingPanel(bool* pOpen);

} // namespace EngineEditor

#endif // CONTEXT_PANELS_H
