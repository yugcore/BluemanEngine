#include "ContextPanels.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>

namespace EngineEditor {

static void RenderPanelPlaceholderHeader(const char* title, const char* category, const ImVec4& accentColor) {
    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleColor(ImGuiCol_Text, accentColor);
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextUnformatted(title);
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::TextColored(pal.textDisabled, "Category: %s", category);
    ImGui::Separator();
    ImGui::Spacing();
}

void RenderMaterialEditorPanel(bool* pOpen) {
    if (!ImGui::Begin("Material Editor", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    const auto& pal = Theme::GetPalette();
    RenderPanelPlaceholderHeader("Material Graph & PBR Node Editor", "Shader / Material Workflow", pal.accent);

    const std::string& asset = EditorState::Get().selectedAssetName;
    if (!asset.empty()) {
        ImGui::TextColored(pal.textPrimary, "Editing Material Asset: %s", asset.c_str());
    } else {
        ImGui::TextColored(pal.textSecondary, "Editing Material: Default_PBR_Material.zmat");
    }

    ImGui::Spacing();
    if (ImGui::BeginTabBar("MaterialGraphTabs")) {
        if (ImGui::BeginTabItem("Graph Canvas")) {
            ImGui::Spacing();
            ImGui::TextDisabled("[ Base Color ] ----> ( PBR Master Node ) <---- [ Normal Map ]");
            ImGui::TextDisabled("[ Roughness  ] ----> ( Metallic 0.85   )");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Material Parameters")) {
            ImGui::Spacing();
            static float baseColor[3] = { 0.8f, 0.2f, 0.2f };
            static float roughness = 0.35f;
            static float metallic = 0.90f;
            ImGui::ColorEdit3("Base Color", baseColor);
            ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
            ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void RenderBlueprintEditorPanel(bool* pOpen) {
    if (!ImGui::Begin("Blueprint / Visual Script", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    const auto& pal = Theme::GetPalette();
    RenderPanelPlaceholderHeader("Blueprint Visual Scripting Workspace", "Logic & Scripting Workflow", ImVec4(0.35f, 0.70f, 0.95f, 1.0f));

    const std::string& asset = EditorState::Get().selectedAssetName;
    ImGui::TextColored(pal.textPrimary, "Target Script: %s", asset.empty() ? "BP_PlayerController.zscript" : asset.c_str());
    ImGui::Spacing();

    if (ImGui::Button("Compile Blueprint")) {
        Logger::Get().Info("[Blueprint] Compiled successfully. 0 warnings.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Event Node")) {
        Logger::Get().Info("[Blueprint] Event node added.");
    }

    ImGui::Spacing();
    ImGui::TextDisabled("Event Graph Nodes:");
    ImGui::BulletText("Event BeginPlay -> Setup Player Input -> Spawn Weapon");
    ImGui::BulletText("Event Tick -> Update Character Movement (DeltaTime)");
    ImGui::End();
}

void RenderAnimationWorkspacePanel(bool* pOpen) {
    if (!ImGui::Begin("Animation Workspace", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    const auto& pal = Theme::GetPalette();
    RenderPanelPlaceholderHeader("Animation Timeline & Skeletal Mesh Rig", "Animation Workflow", ImVec4(0.85f, 0.50f, 0.90f, 1.0f));

    static float currentFrame = 12.0f;
    static bool isPlaying = false;

    ImGui::SliderFloat("Timeline Frame", &currentFrame, 0.0f, 120.0f, "Frame %.0f");
    if (ImGui::Button(isPlaying ? "Pause" : "Play")) isPlaying = !isPlaying;
    ImGui::SameLine();
    ImGui::TextDisabled("Clip: Idle_To_Run_Transition.anim (30 FPS)");

    ImGui::End();
}

void RenderAudioEditorPanel(bool* pOpen) {
    if (!ImGui::Begin("Audio Mixer", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    const auto& pal = Theme::GetPalette();
    RenderPanelPlaceholderHeader("Audio Editor & Soundwave Mixer", "Audio Workflow", ImVec4(0.30f, 0.80f, 0.70f, 1.0f));

    static float masterVolume = 0.85f;
    static float sfxVolume = 1.0f;
    static float musicVolume = 0.60f;

    ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f);
    ImGui::SliderFloat("SFX Bus", &sfxVolume, 0.0f, 1.0f);
    ImGui::SliderFloat("Music Bus", &musicVolume, 0.0f, 1.0f);

    ImGui::End();
}

void RenderProfilerPanel(bool* pOpen) {
    if (!ImGui::Begin("Profiler", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    const auto& pal = Theme::GetPalette();
    RenderPanelPlaceholderHeader("Realtime CPU/GPU Performance Profiler", "Performance Analysis", ImVec4(0.95f, 0.40f, 0.40f, 1.0f));

    ImGui::TextColored(pal.textPrimary, "Frame Time: 6.94 ms (144.1 FPS)");
    ImGui::TextDisabled("CPU Main Thread: 2.10 ms | Render Thread: 1.85 ms | GPU Execute: 2.99 ms");
    ImGui::Separator();
    ImGui::TextUnformatted("Subsystem Timeline:");
    ImGui::BulletText("Shadow Pass: 0.82 ms");
    ImGui::BulletText("Nanite Cluster Cull: 0.45 ms");
    ImGui::BulletText("DXR Raytraced Reflections: 1.20 ms");
    ImGui::BulletText("Post-Process & Composition: 0.52 ms");

    ImGui::End();
}

void RenderRenderDocPanel(bool* pOpen) {
    if (!ImGui::Begin("RenderDoc Integrator", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("RenderDoc Frame Capture", "Graphics Debugging", ImVec4(0.90f, 0.65f, 0.20f, 1.0f));
    if (ImGui::Button("Capture Frame", ImVec2(160.0f, 32.0f))) {
        Logger::Get().Info("[RenderDoc] Triggered frame capture #004.");
    }
    ImGui::TextDisabled("Status: Attached to DX12 Swapchain (Device: D3D12)");
    ImGui::End();
}

void RenderGpuDebuggerPanel(bool* pOpen) {
    if (!ImGui::Begin("GPU Debugger", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("GPU Pipeline State & Wave Inspector", "Graphics Debugging", ImVec4(0.70f, 0.50f, 0.95f, 1.0f));
    ImGui::TextDisabled("Pipeline State Object: PSO_PBR_Opaque_DXR");
    ImGui::TextDisabled("Active Root Signature: RS_Global_DescriptorTable_64");
    ImGui::End();
}

void RenderSequencerPanel(bool* pOpen) {
    if (!ImGui::Begin("Sequencer / Timeline", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Cinematics Sequencer & Camera Tracks", "Cinematics Workflow", ImVec4(0.85f, 0.45f, 0.65f, 1.0f));
    ImGui::TextDisabled("Shot 010_Intro_Pan (00:00:00 - 00:05:00)");
    ImGui::End();
}

void RenderAssetRegistryPanel(bool* pOpen) {
    if (!ImGui::Begin("Asset Registry", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Internal Asset Registry Database", "Technical Tools", Theme::GetPalette().textSecondary);
    ImGui::TextDisabled("Registered Assets: 1,482 Cooked Packages");
    ImGui::TextDisabled("Dependency Cache: Synchronized");
    ImGui::End();
}

void RenderMemoryProfilerPanel(bool* pOpen) {
    if (!ImGui::Begin("Memory Profiler", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("VRAM / RAM Allocation Breakdown", "Optimization Tools", ImVec4(0.40f, 0.75f, 0.85f, 1.0f));
    ImGui::TextDisabled("VRAM Allocated: 4.82 GB / 16.00 GB");
    ImGui::TextDisabled("RAM Allocated:  3.10 GB / 32.00 GB");
    ImGui::End();
}

void RenderPackageManagerPanel(bool* pOpen) {
    if (!ImGui::Begin("Package Manager", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Engine Packages & Dependencies", "Management Tools", Theme::GetPalette().textPrimary);
    ImGui::TextDisabled("Blueman Core SDK v2.4.0 (Up to date)");
    ImGui::End();
}

void RenderPluginManagerPanel(bool* pOpen) {
    if (!ImGui::Begin("Plugin Manager", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Installed Editor Plugins", "Management Tools", Theme::GetPalette().accent);
    static bool enableDXR = true;
    static bool enableDLSS = true;
    ImGui::Checkbox("DirectX Raytracing (DXR) Plugin", &enableDXR);
    ImGui::Checkbox("NVIDIA DLSS 3.5 Upscaler Plugin", &enableDLSS);
    ImGui::End();
}

void RenderLocalizationPanel(bool* pOpen) {
    if (!ImGui::Begin("Localization", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Localization & String Tables", "Specialized Tools", Theme::GetPalette().textSecondary);
    ImGui::TextDisabled("Active Target Culture: en-US");
    ImGui::End();
}

void RenderConsoleVariablesPanel(bool* pOpen) {
    if (!ImGui::Begin("Console Variables", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("Engine Console Variables (CVar)", "Debugging", ImVec4(0.30f, 0.85f, 0.40f, 1.0f));
    static char cvarFilter[64] = "";
    ImGui::InputText("Search CVars", cvarFilter, sizeof(cvarFilter));
    ImGui::TextDisabled("r.Nanite = 1");
    ImGui::TextDisabled("r.RayTracing.Shadows = 1");
    ImGui::TextDisabled("r.VolumetricFog = 1");
    ImGui::End();
}

void RenderNavigationBuilderPanel(bool* pOpen) {
    if (!ImGui::Begin("Navigation Builder", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("NavMesh AI Grid Generation", "AI Workflow", ImVec4(0.90f, 0.70f, 0.30f, 1.0f));
    if (ImGui::Button("Build NavMesh")) {
        Logger::Get().Info("[Navigation] Rebuilt spatial grid in 120ms.");
    }
    ImGui::End();
}

void RenderLightBakingPanel(bool* pOpen) {
    if (!ImGui::Begin("Light Baking", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }
    RenderPanelPlaceholderHeader("DXR Lightmass Bake & Radiosity Cache", "Build Workflow", ImVec4(0.95f, 0.60f, 0.20f, 1.0f));
    if (ImGui::Button("Bake Static Lighting")) {
        Logger::Get().Info("[LightBaking] Direct & Indirect radiosity baking initiated...");
    }
    ImGui::End();
}

} // namespace EngineEditor
