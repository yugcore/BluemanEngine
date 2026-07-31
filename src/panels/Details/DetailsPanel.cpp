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
#include <imgui_internal.h>

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

    // 1. Header Block: Stacked Name & Type Layout with Type Icon + Right-aligned "Edit in C++"
    ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 headerStart = ImGui::GetCursorScreenPos();

    // Determine type icon color and shape
    ImVec2 iconBoxMin = ImVec2(headerStart.x, headerStart.y + 2.0f);
    ImVec2 iconBoxMax = ImVec2(iconBoxMin.x + 16.0f, iconBoxMin.y + 16.0f);

    if (selectedNodeType == "Folder" || selectedNodeName.find("Folder") != std::string::npos) {
        ImU32 folderCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.75f, 0.35f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 3.0f), ImVec2(iconBoxMax.x - 1.0f, iconBoxMax.y - 1.0f), folderCol, 2.0f);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 1.0f), ImVec2(iconBoxMin.x + 7.0f, iconBoxMin.y + 4.0f), folderCol, 1.0f);
    } else if (selectedNodeType == "Light" || selectedNodeName.find("Light") != std::string::npos) {
        ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.96f, 0.82f, 0.28f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.5f, lightCol);
    } else if (selectedNodeType == "Camera" || selectedNodeName.find("Camera") != std::string::npos) {
        ImU32 camCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.85f, 0.50f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 4.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), camCol, 2.0f);
    } else {
        ImU32 meshCol = ImGui::ColorConvertFloat4ToU32(pal.accent);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 2.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), meshCol, 2.0f);
    }

    float textX = Theme::Metrics::panelLeftMargin + 22.0f;

    // Right-aligned "Edit in C++" button aligned directly with top header row
    const char* editCppLabel = "Edit in C++";
    float editBtnWidth = ImGui::CalcTextSize(editCppLabel).x + 20.0f;
    float editBtnX = ImGui::GetWindowWidth() - editBtnWidth - Theme::Metrics::panelLeftMargin;

    if (editBtnX > textX + 100.0f) {
        ImGui::SetCursorScreenPos(ImVec2(editBtnX, headerStart.y + 4.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_Text, pal.accent);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));

        if (ImGui::Button(editCppLabel)) {
            Logger::Get().Info("[Details] Edit in C++ clicked for " + selectedNodeName);
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);
    }

    // Stacked text layout on left side (Name + Type)
    ImGui::SetCursorScreenPos(ImVec2(headerStart.x + 22.0f, headerStart.y));
    ImGui::BeginGroup();

    // Line 1: Object Name (Bold/Semibold)
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.accent, "%s", selectedNodeName.c_str());
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PopFont();

    // Line 2: Type Label (Smaller, Muted Text directly below)
    std::string componentName = selectedNodeType.empty() ? "ActorComponent" : (selectedNodeType + "Component");
    ImGui::TextColored(pal.textDisabled, "%s", componentName.c_str());

    ImGui::EndGroup();

    ImGui::Spacing();
    ImGui::Separator();

    auto RenderCollapsibleHeader = [&](const char* label, bool defaultOpen = true) -> bool {
        static std::unordered_map<std::string, bool> s_States;
        if (s_States.find(label) == s_States.end()) s_States[label] = defaultOpen;
        bool& open = s_States[label];

        ImGui::Spacing();

        ImVec2 hMin = ImGui::GetCursorScreenPos();
        float availW = ImGui::GetContentRegionAvail().x;
        float hHeight = 26.0f;
        ImVec2 hMax = ImVec2(hMin.x + availW, hMin.y + hHeight);

        ImGui::ItemSize(ImVec2(availW, hHeight));
        ImGui::ItemAdd(ImRect(hMin, hMax), ImGui::GetID(label));

        bool hovered = ImGui::IsItemHovered();
        bool clicked = ImGui::IsItemClicked();

        if (clicked) {
            open = !open;
        }

        // Left-aligned header background fill
        ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(hovered ? pal.bgElevated : pal.bgHeader);
        dl->AddRectFilled(hMin, hMax, bgCol, 2.0f);

        // Vector Triangle Disclosure Caret
        float caretX = hMin.x + Theme::Metrics::panelLeftMargin;
        float centerY = hMin.y + hHeight * 0.5f;
        ImU32 caretCol = ImGui::ColorConvertFloat4ToU32(pal.textSecondary);

        if (open) {
            // Down-pointing triangle
            ImVec2 p1(caretX, centerY - 3.0f);
            ImVec2 p2(caretX + 8.0f, centerY - 3.0f);
            ImVec2 p3(caretX + 4.0f, centerY + 3.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretCol);
        } else {
            // Right-pointing triangle
            ImVec2 p1(caretX + 2.0f, centerY - 4.0f);
            ImVec2 p2(caretX + 7.0f, centerY);
            ImVec2 p3(caretX + 2.0f, centerY + 4.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretCol);
        }

        // Left-aligned section header title
        ImVec2 textPos = ImVec2(caretX + 16.0f, hMin.y + (hHeight - ImGui::GetTextLineHeight()) * 0.5f);
        dl->AddText(textPos, ImGui::ColorConvertFloat4ToU32(pal.textPrimary), label);

        ImGui::Spacing();
        return open;
    };

    // 2. Transition (Transform Block)
    bool transitionOpen = RenderCollapsibleHeader("Transition", true);

    if (transitionOpen) {
        ImGui::Indent(Theme::Metrics::panelLeftMargin);
        ImGui::Spacing();

        auto& transform = EditorState::Get().activeTransform;

        Widgets::RenderVector3PropertyRow("Location", transform.location, 0.0f);
        ImGui::Spacing();
        Widgets::RenderVector3PropertyRow("Rotation", transform.rotation, 0.0f);
        ImGui::Spacing();
        Widgets::RenderVector3PropertyRow("Scale", transform.scale, 1.0f, &transform.lockAspect);

        ImGui::Spacing();
        ImGui::Unindent(Theme::Metrics::panelLeftMargin);
    }

    // 3. Component-Specific Section
    if (selectedNodeName == "SkyAtmosphere" || selectedNodeType == "SkyAtmosphere") {
        bool skyOpen = RenderCollapsibleHeader("Sky Atmosphere", true);

        if (skyOpen) {
            ImGui::Indent(Theme::Metrics::panelLeftMargin);
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
            ImGui::Unindent(Theme::Metrics::panelLeftMargin);
        }
    } else if (selectedNodeType == "Light" || selectedNodeName == "SunLight" || selectedNodeName == "SkyLight") {
        bool lightOpen = RenderCollapsibleHeader("Directional Light Component", true);

        if (lightOpen) {
            ImGui::Indent(Theme::Metrics::panelLeftMargin);
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
            ImGui::Unindent(Theme::Metrics::panelLeftMargin);
        }
    } else {
        bool actorOpen = RenderCollapsibleHeader("Actor Component", true);

        if (actorOpen) {
            ImGui::Indent(Theme::Metrics::panelLeftMargin);
            ImGui::Spacing();
            ImGui::TextDisabled("Generic Component Properties for %s", selectedNodeName.c_str());
            ImGui::Spacing();
            static bool isStatic = true;
            ImGui::Checkbox("Static Mobility", &isStatic);
            ImGui::Spacing();
            ImGui::Unindent(Theme::Metrics::panelLeftMargin);
        }
    }

    ImGui::End();
}

} // namespace EngineEditor
