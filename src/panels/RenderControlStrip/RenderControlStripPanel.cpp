#include "RenderControlStripPanel.h"
#include "core/EditorState.h"
#include "core/Logger.h"
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
            Logger::Get().Info(std::string("[RenderControlStrip] Quality Preset set to ") + label);
        }
        ImGui::PopStyleColor(2);
    };

    presetBtn("Low", 0); ImGui::SameLine();
    presetBtn("Medium", 1); ImGui::SameLine();
    presetBtn("High", 2); ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
    if (ImGui::Button("Save", ImVec2(70.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[RenderControlStrip] Settings saved.");
    }
    ImGui::PopStyleColor(2);

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
        ImGui::Checkbox("Ray Traced Global Illumination (RTGI) [Quality: Ultra, Bounces: 4]", &settings.rtGI);
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

    // --- Quick Isolation Tests ---
    if (RenderFlatHeader("Quick Isolating Tests (MRQ Floating)", true)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();
        
        ImGui::Checkbox("Geometry isolation (Collapse scene occlusion with Dolly)", &settings.isoGeometry);
        ImGui::Checkbox("Texture isolation (Disable alt on route / safe route)", &settings.isoTextures);
        ImGui::Checkbox("Lighting isolation (Enable sun lightmaps sync)", &settings.isoLighting);
        ImGui::Checkbox("Shadow isolation (Disable double shadow map process)", &settings.isoShadows);
        ImGui::Checkbox("DXR Ray Tracing isolation (Disable DXR ray tracing)", &settings.isoDXR);
        ImGui::Checkbox("Material isolation (Use solid color accent)", &settings.isoMaterials);
        ImGui::Checkbox("Mesh isolation (Replace with 12-sample mesh)", &settings.isoMeshShader);
        ImGui::Checkbox("Render isolation (Fully show rendered authority)", &settings.isoPostProcess);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    ImGui::End();
}

} // namespace EngineEditor
