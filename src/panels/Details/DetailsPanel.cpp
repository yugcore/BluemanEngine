#include "DetailsPanel.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/Logger.h"
#include "widgets/PropertyRow.h"
#include "theme/Fonts.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <unordered_map>
#include <imgui.h>

namespace EngineEditor {

void RenderDetailsPanel(bool* pOpen) {
    if (!ImGui::Begin("Details", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const std::string& selectedNodeName = EditorState::Get().selectedNodeName;
    const std::string& selectedNodeType = EditorState::Get().selectedNodeType;

    const auto& pal = Theme::GetPalette();

    if (selectedNodeName.empty()) {
        ImGui::Dummy(ImVec2(0.0f, Theme::Metrics::sectionIndent));
        ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
        ImGui::TextColored(pal.textDisabled, "No selection.");
        ImGui::End();
        return;
    }

    // 1. Header showing panel context / Instance Name
    ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
    if (Theme::GetFontAtlas().panelTitleFont)
        ImGui::PushFont(Theme::GetFontAtlas().panelTitleFont);
    ImGui::TextColored(pal.accent, "%s (Instance)", selectedNodeName.c_str());
    if (Theme::GetFontAtlas().panelTitleFont)
        ImGui::PopFont();

    ImGui::Spacing();

    // 2. Component breadcrumb
    ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
    std::string componentName = selectedNodeType.empty() ? "ActorComponent" : (selectedNodeType + "Component");
    ImGui::TextColored(pal.textPrimary, "%s (%s)", componentName.c_str(), componentName.c_str());

    float rightBtnX = ImGui::GetWindowWidth() - 110.0f;
    if (rightBtnX > ImGui::GetCursorPosX()) {
        ImGui::SameLine(rightBtnX);
    } else {
        ImGui::SameLine();
    }
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
    if (ImGui::SmallButton("Edit in C++")) {
        Logger::Get().Info("[Details] Edit in C++ clicked for " + selectedNodeName);
    }
    ImGui::PopStyleColor(3);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

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

    // 3. Transition (Transform Block)
    bool transitionOpen = RenderFlatHeader("Transition", true);

    if (transitionOpen) {
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
        bool skyOpen = RenderFlatHeader("Sky Atmosphere", true);

        if (skyOpen) {
            ImGui::Indent(Theme::Metrics::sectionIndent);
            ImGui::Spacing();

            auto& skyProps = EditorState::Get().skyAtmosphereProps;

            ImGui::Columns(2, "##SkyProps", false);
            ImGui::SetColumnWidth(0, Theme::Metrics::labelColumnWidth * 2.0f);

            ImGui::TextUnformatted("Rayleigh Scattering (10\xE2\x81\xBB\xE2\x81\xB6 m\xE2\x81\xBB\xC2\xB9)");
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

            ImGui::TextUnformatted("Aerial Perspective Scale");
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
        bool lightOpen = RenderFlatHeader("Directional Light Component", true);

        if (lightOpen) {
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
        bool actorOpen = RenderFlatHeader("Actor Component", true);

        if (actorOpen) {
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
