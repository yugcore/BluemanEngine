#include "DetailsPanel.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/Logger.h"
#include "widgets/PropertyRow.h"
#include "theme/Fonts.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

void RenderDetailsPanel(bool* pOpen) {
    if (!ImGui::Begin("Details", pOpen)) {
        ImGui::End();
        return;
    }

    const std::string& selectedNodeName = EditorState::Get().selectedNodeName;
    const std::string& selectedNodeType = EditorState::Get().selectedNodeType;

    if (selectedNodeName.empty()) {
        ImGui::TextDisabled("No selection.");
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // 1. Header: Instance Name + Type
    if (Theme::GetFontAtlas().panelTitleFont)
        ImGui::PushFont(Theme::GetFontAtlas().panelTitleFont);
    ImGui::TextColored(pal.accent, "%s (Instance)", selectedNodeName.c_str());
    if (Theme::GetFontAtlas().panelTitleFont)
        ImGui::PopFont();

    ImGui::Spacing();

    // 2. Component breadcrumb
    ImGui::TextColored(pal.textSecondary, "\xE2\x96\xB8");
    ImGui::SameLine();
    
    // Determine component name
    std::string componentName = selectedNodeType.empty() ? "ActorComponent" : (selectedNodeType + "Component");
    ImGui::TextColored(pal.textPrimary, "%s (%s)", componentName.c_str(), componentName.c_str());

    ImGui::SameLine(ImGui::GetWindowWidth() - 100.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
    if (ImGui::SmallButton("Edit in C++")) {
        Logger::Get().Info("[Details] Edit in C++ clicked for " + selectedNodeName);
    }
    ImGui::PopStyleColor(2);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. Transition (Transform Block)
    if (ImGui::CollapsingHeader("Transition", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(Theme::Metrics::sectionIndent);
        ImGui::Spacing();

        auto& transform = EditorState::Get().activeTransform;

        Widgets::RenderVector3PropertyRow("Location", transform.location, 0.0f);
        ImGui::Spacing();
        Widgets::RenderVector3PropertyRow("Rotation", transform.rotation, 0.0f);
        ImGui::Spacing();
        Widgets::RenderVector3PropertyRow("Scale", transform.scale, 1.0f, &transform.lockAspect);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::sectionIndent);
    }

    ImGui::Spacing();

    // 4. Component-Specific Section
    if (selectedNodeName == "SkyAtmosphere" || selectedNodeType == "SkyAtmosphere") {
        if (ImGui::CollapsingHeader("Sky Atmosphere", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(Theme::Metrics::sectionIndent);
            ImGui::Spacing();

            auto& skyProps = EditorState::Get().skyAtmosphereProps;

            // Property rows with label-value layout
            ImGui::Columns(2, "##SkyProps", false);
            ImGui::SetColumnWidth(0, 260.0f);

            ImGui::TextUnformatted("Rayleigh Scattering Coefficient (10\xE2\x81\xBB\xE2\x81\xB6 m\xE2\x81\xBB\xC2\xB9)");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##RayleighScattering", &skyProps.rayleighScattering, 0.001f, 0.000f, 1.000f, "%.4f");
            ImGui::NextColumn();

            ImGui::TextUnformatted("Aerosol Scattering Scale");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##AerosolScattering", &skyProps.aerosolScattering, 0.001f, 0.000f, 1.000f, "%.1f");
            ImGui::NextColumn();

            ImGui::TextUnformatted("Aerosol Absorption Scale");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##AerosolAbsorption", &skyProps.aerosolAbsorption, 0.001f, 0.000f, 1.000f, "%.1f");
            ImGui::NextColumn();

            ImGui::TextUnformatted("Atmosphere Height");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##AtmosphereHeight", &skyProps.atmosphereHeightKm, 0.5f, 1.0f, 100.0f, "%.1f km");
            ImGui::NextColumn();

            ImGui::TextUnformatted("Aerial Perspective View Distance Scale");
            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##AerialPerspective", &skyProps.aerialPerspectiveDistanceScale, 0.05f, 0.1f, 10.0f, "%.2f");

            ImGui::Columns(1);
            ImGui::Spacing();

            // Toggles
            static bool showTransition = true;
            ImGui::Checkbox("Transition", &showTransition);
            ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
            
            static bool showSkyAtm = true;
            ImGui::Checkbox("Sky Atmosphere", &showSkyAtm);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::BeginTabBar("SkyAtmosphereTabs")) {
                if (ImGui::BeginTabItem("Ray Tracing")) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Ray Traced Atmospheric Single/Multi-Scattering Active");
                    static bool rtSkyIllum = true;
                    ImGui::Checkbox("RT Sky Atmosphere Illumination", &rtSkyIllum);
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Global Illumination")) {
                    ImGui::Spacing();
                    ImGui::TextDisabled("Sky Light Realtime Capture & Luminance Integration");
                    static float skyIntensity = 1.0f;
                    ImGui::SliderFloat("Sky Luminance Multiplier", &skyIntensity, 0.0f, 10.0f);
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }

            ImGui::Spacing();
            ImGui::Unindent(Theme::Metrics::sectionIndent);
        }
    } else if (selectedNodeType == "Light" || selectedNodeName == "SunLight" || selectedNodeName == "SkyLight") {
        if (ImGui::CollapsingHeader("Directional Light Component", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(Theme::Metrics::sectionIndent);
            ImGui::Spacing();
            static float intensity = 100000.0f;
            static float lightColor[3] = { 1.0f, 0.95f, 0.85f };

            ImGui::TextUnformatted("Intensity (Lux):");
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::DragFloat("##LightIntensity", &intensity, 500.0f, 0.0f, 500000.0f, "%.0f Lux");

            ImGui::TextUnformatted("Light Color:");
            ImGui::SetNextItemWidth(-1.0f);
            ImGui::ColorEdit3("##LightColor", lightColor);

            ImGui::Spacing();
            ImGui::Unindent(Theme::Metrics::sectionIndent);
        }
    } else {
        if (ImGui::CollapsingHeader("Actor Component", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Indent(Theme::Metrics::sectionIndent);
            ImGui::Spacing();
            ImGui::TextDisabled("Generic Component Properties for %s", selectedNodeName.c_str());
            static bool isStatic = true;
            ImGui::Checkbox("Static Mobility", &isStatic);
            ImGui::Spacing();
            ImGui::Unindent(Theme::Metrics::sectionIndent);
        }
    }

    ImGui::End();
}

} // namespace EngineEditor
