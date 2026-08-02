#include "RenderControlStripPanel.h"
#include "render/ZeGFXAdapter.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <unordered_map>
#include <imgui.h>
#include <cstdio>

namespace EngineEditor {

void RenderRenderControlStripPanel(bool* pOpen) {
    auto& settings = EditorState::Get().settings;
    auto& stats = EditorState::Get().stats;

    bool* openPtr = pOpen ? pOpen : &settings.showRenderControlStrip;
    if (!*openPtr) return;

    ImGui::SetNextWindowPos(ImVec2(280.0f, 110.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(420.0f, 640.0f), ImGuiCond_FirstUseEver);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;

    if (!ImGui::Begin("Render Control Strip", openPtr, flags)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // --- GPU Info Card ---
    ImGui::PushStyleColor(ImGuiCol_ChildBg, pal.bgBase);
    ImGui::PushStyleColor(ImGuiCol_Border, pal.borderSubtle);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);

    if (ImGui::BeginChild("RenderInfoBox", ImVec2(0, 125.0f), true)) {
        ImGui::TextColored(pal.accent, "GPU: %s", stats.gpuName.c_str());
        ImGui::Spacing();
        
        char vramBuf[64];
        snprintf(vramBuf, sizeof(vramBuf), "VRAM Usage: %.1f GB / %.1f GB", stats.vramUsedGB, stats.vramTotalGB);
        ImGui::TextColored(pal.textSecondary, "%s", vramBuf);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, pal.accent);
        ImGui::ProgressBar(stats.vramUsedGB / stats.vramTotalGB, ImVec2(-1.0f, 4.0f), "");
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::TextColored(pal.textSecondary, "DR12 Hardware Ray Tracing (DXR) Active");
        
        if (Theme::GetFontAtlas().secondaryFont)
            ImGui::PushFont(Theme::GetFontAtlas().secondaryFont);
        ImGui::TextColored(pal.textDisabled, "%s", stats.csrPolicy.c_str());
        if (Theme::GetFontAtlas().secondaryFont)
            ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);

    ImGui::Spacing();

    // --- Quality Presets ---
    ImGui::TextUnformatted("Global Quality Presets:");
    ImGui::Spacing();
    
    auto presetBtn = [&](const char* label, int index) {
        bool isActive = (settings.qualityPreset == index);
        if (isActive) {
            ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
            ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
            ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
        }
        if (ImGui::Button(label, ImVec2(70.0f, Theme::Metrics::rowHeight))) {
            settings.qualityPreset = index;
            if (index == 0) { // Low (Ultra fast performance mode)
                settings.rtGI = false;
                settings.rtAO = false;
                settings.rtReflections = false;
                settings.fog.enableVolumetric = false;
                settings.shadow.cascadeResolution = 1024;
                settings.shadow.cascadeCount = 2;
            } else if (index == 1) { // Medium (Balanced PBR)
                settings.rtGI = false;
                settings.rtAO = false;
                settings.rtReflections = true;
                settings.fog.enableVolumetric = true;
                settings.shadow.cascadeResolution = 2048;
                settings.shadow.cascadeCount = 4;
            } else if (index == 2) { // High (Full AAA DXR)
                settings.rtGI = true;
                settings.rtAO = true;
                settings.rtReflections = true;
                settings.fog.enableVolumetric = true;
                settings.shadow.cascadeResolution = 2048;
                settings.shadow.cascadeCount = 4;
            } else if (index == 3) { // Ultra (Cinematic 4K CSM)
                settings.rtGI = true;
                settings.rtAO = true;
                settings.rtReflections = true;
                settings.fog.enableVolumetric = true;
                settings.shadow.cascadeResolution = 4096;
                settings.shadow.cascadeCount = 4;
            }
            Logger::Get().Info(std::string("[RenderControlStrip] Quality Preset set to ") + label);
        }
        ImGui::PopStyleColor(2);
    };

    presetBtn("Low", 0); ImGui::SameLine();
    presetBtn("Medium", 1); ImGui::SameLine();
    presetBtn("High", 2); ImGui::SameLine();
    presetBtn("Ultra", 3);

    ImGui::Spacing();
    const char* debugModes[] = {
        "0: Default (Lit)",
        "1: Unlit",
        "2: Lighting Only",
        "3: World Normals",
        "4: Roughness",
        "5: Metallic",
        "6: Albedo (Base Color)",
        "7: Scene Depth",
        "8: Shadow Cascades",
        "9: SSAO Buffer",
        "10: Volumetric Fog"
    };
    int currentDebug = settings.lightingDebugMode;
    if (ImGui::Combo("Buffer Visualization", &currentDebug, debugModes, 11)) {
        ZeGFXAdapter::Get().SetLightingDebugMode(currentDebug);
    }

    ImGui::Checkbox("Enable VSync", &settings.enableVSync);

    auto RenderFlatHeader = [&](const char* label, bool defaultOpen = true) -> bool {
        static std::unordered_map<std::string, bool> s_States;
        if (s_States.find(label) == s_States.end()) s_States[label] = defaultOpen;
        bool& open = s_States[label];

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);

        if (Theme::GetFontAtlas().sectionHeaderFont)
            ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);

        float availW = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button(label, ImVec2(availW, 30.0f))) {
            open = !open;
        }

        if (Theme::GetFontAtlas().sectionHeaderFont)
            ImGui::PopFont();

        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(2);
        return open;
    };

    ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

    // --- Hardware Ray Tracing (DXR) ---
    if (RenderFlatHeader("Hardware Ray Tracing (DXR)", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        ImGui::Checkbox("Ray Traced Global Illumination (RTGI) [ZeGI Probes]", &settings.rtGI);
        if (settings.rtGI) {
            ImGui::Indent();
            ImGui::SetNextItemWidth(180.0f);
            ImGui::SliderInt("Probe Ray Count", &settings.giRaysPerProbe, 32, 256);
            ImGui::SetNextItemWidth(180.0f);
            ImGui::SliderInt("Probes Updated / Frame", &settings.giProbesUpdatedPerFrame, 16, 128);
            ImGui::Unindent();
            ImGui::Spacing();
        }
        ImGui::Checkbox("Ray Traced Ambient Occlusion (RTAO)", &settings.rtAO);
        ImGui::Checkbox("Ray Traced Reflections (RTR)", &settings.rtReflections);

        ImGui::Spacing();
        char dsrBuf[64]; snprintf(dsrBuf, sizeof(dsrBuf), "%.2fx", settings.dsrScale);
        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderFloat("##DSRScale", &settings.dsrScale, 0.50f, 2.00f, dsrBuf);
        ImGui::SameLine();
        ImGui::Checkbox("DSR Resolution Scale", &settings.dsrEnabled);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- World Partition & Spatial Grid ---
    if (RenderFlatHeader("World Partition & Spatial Grid Streaming", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Cell Size (m)", &settings.cellSizeMeters, 50.0f, 500.0f, "%.0fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Streaming Radius (m)", &settings.streamingRadiusMeters, 100.0f, 1000.0f, "%.0fm");

        ImGui::TextDisabled("Active Streamed Level: %d | Total Spatial Scale: %dm", settings.activeStreamedLevel, settings.totalSpatialScale);
        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- Nanite & Mesh Shader ---
    if (RenderFlatHeader("Nanite Virtual Geometry & Mesh Shader Pipeline", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        ImGui::Checkbox("Enable Virtual Geometry Clusters", &settings.naniteClusterCulling);
        ImGui::Checkbox("BLT Processor Decimation Orders", &settings.naniteFrustumCulling);
        ImGui::Checkbox("Mesh Shader Acceleration (Output MeshShaders)", &settings.meshShaderPipeline);

        ImGui::TextDisabled("Max Key Traversals Levels: 0 | Shadow Stencils: 0");
        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- High-Density Foliage & GPU Culling (ExecuteIndirect) ---
    if (RenderFlatHeader("High-Density Foliage & GPU Culling (ExecuteIndirect)", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        ImGui::Checkbox("Enable GPU Compute Culling", &settings.foliage.enableGPUCulling);
        ImGui::Checkbox("Enable ExecuteIndirect Dispatch", &settings.foliage.enableExecuteIndirect);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Foliage Cull Distance (m)", &settings.foliage.cullDistanceMeters, 100.0f, 2000.0f, "%.0fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Tree Density Multiplier", &settings.foliage.treeDensity, 0.1f, 3.0f, "%.1fx");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Grass Density Multiplier", &settings.foliage.grassDensity, 0.1f, 5.0f, "%.1fx");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderInt("Max Instance Capacity", &settings.foliage.maxFoliageInstances, 10000, 500000);

        ImGui::TextDisabled("Active Foliage Instances: 250,000 | Indirect Buffers: Active");
        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- 1KM Heightmap Terrain System ---
    if (RenderFlatHeader("1KM Heightmap Terrain System", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderInt("Grid Resolution", &settings.terrain.gridWidth, 256, 1024);
        settings.terrain.gridHeight = settings.terrain.gridWidth;

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Cell Size (m)", &settings.terrain.cellSize, 1.0f, 5.0f, "%.1fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Height Scale (m)", &settings.terrain.heightScale, 10.0f, 150.0f, "%.0fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderInt("Max Terrain LOD Level", &settings.terrain.maxLodLevel, 1, 6);

        float terrainCoverageKm = (settings.terrain.gridWidth * settings.terrain.cellSize) / 1000.0f;
        ImGui::TextDisabled("Terrain Dimension: %.2f km x %.2f km | Heightfield: Active", terrainCoverageKm, terrainCoverageKm);

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
        if (ImGui::Button("Spawn 1KM Forest Terrain Chunk", ImVec2(240.0f, Theme::Metrics::rowHeight))) {
            SceneNode terrainNode;
            terrainNode.name = "Forest_Terrain_Chunk_" + std::to_string(rand() % 100);
            terrainNode.type = SceneNodeType::Terrain;
            SceneGraph::Get().AddNode(terrainNode);
            Logger::Get().Info("[Terrain] Spawned 1KM Forest Terrain Chunk: " + terrainNode.name);
        }
        ImGui::PopStyleColor(2);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- ZeGFX Volumetric Froxel Fog ---
    if (RenderFlatHeader("ZeGFX Volumetric Froxel Fog", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        ImGui::Checkbox("Enable Volumetric Fog (Lighting Injection + Z-Sum)", &settings.fog.enableVolumetric);
        
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Fog Density", &settings.fog.density, 0.001f, 0.20f, "%.3f");
        
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Anisotropy (G-Factor)", &settings.fog.anisotropy, -0.90f, 0.90f, "%.2f");
        
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Max Volume Distance", &settings.fog.maxDistance, 10.0f, 500.0f, "%.0fm");

        ImGui::ColorEdit3("Fog Scattering Color", settings.fog.color);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- ZeGFX Cascaded Shadow Maps (CSM) ---
    if (RenderFlatHeader("ZeGFX Cascaded Shadow Maps (CSM)", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        
        const char* resOptions[] = { "512x512", "1024x1024", "2048x2048", "4096x4096" };
        int resIdx = (settings.shadow.cascadeResolution == 512) ? 0 : (settings.shadow.cascadeResolution == 1024) ? 1 : (settings.shadow.cascadeResolution == 4096) ? 3 : 2;
        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::Combo("Cascade Resolution", &resIdx, resOptions, 4)) {
            settings.shadow.cascadeResolution = (resIdx == 0) ? 512 : (resIdx == 1) ? 1024 : (resIdx == 3) ? 4096 : 2048;
        }

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderInt("Cascade Count", &settings.shadow.cascadeCount, 1, 4);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Max Shadow Distance", &settings.shadow.maxDistance, 20.0f, 500.0f, "%.0fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Constant Bias", &settings.shadow.constantBias, 0.00001f, 0.0050f, "%.5f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Slope Bias", &settings.shadow.slopeBias, 0.0001f, 0.0100f, "%.4f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Normal Bias", &settings.shadow.normalBias, 0.01f, 2.00f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Filter Softness (PCF)", &settings.shadow.filterSoftness, 0.10f, 5.00f, "%.2f");

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- ZeGFX Ambient Occlusion ---
    if (RenderFlatHeader("ZeGFX Ambient Occlusion (AO)", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();

        const char* aoModes[] = { "SSAO (Screen-Space)", "GTAO (Ground-Truth)", "DXR Raytraced AO" };
        ImGui::SetNextItemWidth(200.0f);
        ImGui::Combo("AO Algorithm", &settings.ao.mode, aoModes, 3);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("AO Sample Radius", &settings.ao.radius, 0.2f, 5.0f, "%.2fm");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("AO Intensity", &settings.ao.intensity, 0.0f, 3.0f, "%.2f");

        ImGui::Checkbox("Temporal Accumulation Filter", &settings.ao.temporalFiltering);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- ZeGFX Post-Processing & Tonemapping ---
    if (RenderFlatHeader("ZeGFX Post-Processing & Tonemapping", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();

        const char* operators[] = { "ACES (Filmic)", "Reinhard", "Uncharted 2", "Linear (Pass-Through)" };
        ImGui::SetNextItemWidth(200.0f);
        ImGui::Combo("Tonemap Operator", &settings.postFX.tonemapOperator, operators, 4);

        ImGui::Checkbox("Auto-Exposure (Histogram Compute)", &settings.postFX.autoExposure);
        if (!settings.postFX.autoExposure) {
            ImGui::SetNextItemWidth(200.0f);
            ImGui::SliderFloat("Manual Exposure EV", &settings.postFX.exposureEV, -4.0f, 4.0f, "%.1f EV");
        }

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Contrast", &settings.postFX.contrast, 0.5f, 2.0f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Saturation", &settings.postFX.saturation, 0.0f, 2.0f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Color Temp (K)", &settings.postFX.temperature, 2000.0f, 12000.0f, "%.0fK");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Tint Offset", &settings.postFX.tint, -1.0f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Bloom Intensity", &settings.postFX.bloomIntensity, 0.0f, 2.0f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Bloom Threshold", &settings.postFX.bloomThreshold, 0.1f, 3.0f, "%.2f");

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    // --- ZeGFX PBR Material Inspector ---
    if (RenderFlatHeader("ZeGFX Material PBR Texture Channels", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();

        auto& mat = settings.activeMaterial;
        ImGui::TextColored(pal.accent, "Active Material: %s", mat.materialName.c_str());
        ImGui::Spacing();

        ImGui::ColorEdit4("Base Color / Tint", mat.baseColor);

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Roughness Factor", &mat.roughness, 0.0f, 1.0f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Metallic Factor", &mat.metallic, 0.0f, 1.0f, "%.2f");

        ImGui::SetNextItemWidth(200.0f);
        ImGui::SliderFloat("Specular Level", &mat.specular, 0.0f, 1.0f, "%.2f");

        ImGui::Spacing();
        ImGui::TextDisabled("Texture Map Slots:");
        ImGui::Text("Albedo Map:    %s", mat.albedoMap.c_str());
        ImGui::Text("Normal Map:    %s", mat.normalMap.c_str());
        ImGui::Text("Roughness Map: %s", mat.roughnessMap.c_str());
        ImGui::Text("Metallic Map:  %s", mat.metallicMap.c_str());
        ImGui::Text("Emissive Map:  %s", mat.emissiveMap.c_str());
        ImGui::Text("Occlusion Map: %s", mat.occlusionMap.c_str());

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    ImGui::End();
}

} // namespace EngineEditor
