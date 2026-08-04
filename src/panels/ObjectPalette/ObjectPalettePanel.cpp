#include "ObjectPalettePanel.h"
#include "engine/scene/SceneGraph.h"
#include "core/EditorState.h"
#include "engine/core/Logger.h"
#include "widgets/SearchBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

namespace EngineEditor {

struct PaletteItem {
    std::string name;
    std::string category;
    std::string description;
    SceneNodeType type;
    const char* badgeStr;
    std::string meshPath = "";
    int defaultLightType = -1; // -1: N/A, 0: Directional, 1: Point, 2: Spot
};

static char s_PaletteSearch[128] = "";

static const std::vector<PaletteItem> s_PaletteItems = {
    { "32x32 Terrain Ground", "Environment", "32m x 32m solid terrain ground mesh", SceneNodeType::Terrain, "[Terrain]", "Engine/DefaultTerrain32x32", -1 },
    { "Foliage Cluster", "Environment", "High-density instanced trees, bushes & grass", SceneNodeType::FoliageCluster, "[Foliage]", "Engine/DefaultCone", -1 },
    { "Trail Path Waypoint", "Environment", "Forest trail path marker node", SceneNodeType::PathPoint, "[Path]", "Engine/DefaultPlane", -1 },

    { "Cube Mesh", "Primitives", "Standard 3D unit cube mesh", SceneNodeType::Actor, "[Cube]", "primitives/cube.zmesh", -1 },
    { "Sphere Mesh", "Primitives", "UV sphere mesh primitive", SceneNodeType::Actor, "[Sphere]", "primitives/sphere.zmesh", -1 },
    { "Cylinder Mesh", "Primitives", "Subdivided cylinder mesh primitive", SceneNodeType::Actor, "[Cyl]", "primitives/cylinder.zmesh", -1 },
    { "Plane Mesh", "Primitives", "Flat ground plane geometry", SceneNodeType::Actor, "[Plane]", "primitives/plane.zmesh", -1 },
    { "Cone Mesh", "Primitives", "Conical geometry primitive", SceneNodeType::Actor, "[Cone]", "primitives/cone.zmesh", -1 },

    { "Directional Light", "Lighting", "Sunlight / main directional light source", SceneNodeType::Light, "[Sun]", "", 0 },
    { "Point Light", "Lighting", "Omnidirectional point light source", SceneNodeType::Light, "[Light]", "", 1 },
    { "Spot Light", "Lighting", "Cone-focused spot light source", SceneNodeType::Light, "[Spot]", "", 2 },
    { "Sky Atmosphere", "Lighting", "Rayleigh & Mie atmospheric sky solve", SceneNodeType::SkyAtmosphere, "[Sky]", "", -1 },

    { "Cinematic Camera", "Cameras", "Perspective camera actor with focal length & FOV", SceneNodeType::Camera, "[Cam]", "", -1 },

    { "Audio Source", "Volumes", "Spatial 3D audio emitter volume", SceneNodeType::Audio, "[Audio]", "", -1 },
    { "Box Collision", "Volumes", "AABB collision trigger volume", SceneNodeType::Component, "[Vol]", "", -1 },
    { "Post Process Volume", "Volumes", "Tone mapping and bloom region volume", SceneNodeType::Component, "[PP]", "", -1 }
};

void RenderObjectPalettePanel(bool* pOpen) {
    if (!ImGui::Begin("Object Palette", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    // Top Search Bar
    float availW = ImGui::GetContentRegionAvail().x;
    Widgets::RenderSearchBar("##PaletteSearch", s_PaletteSearch, sizeof(s_PaletteSearch), "Search objects...", availW);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Category Tabs / Filter Buttons
    static int s_SelectedCategoryIndex = 0; // 0: All, 1: Environment, 2: Primitives, 3: Lighting, 4: Cameras, 5: Volumes
    const char* categories[] = { "All", "Environment", "Primitives", "Lighting", "Cameras", "Volumes" };

    for (int c = 0; c < 6; ++c) {
        if (c > 0) ImGui::SameLine(0.0f, 4.0f);
        bool isSel = (s_SelectedCategoryIndex == c);
        ImGui::PushStyleColor(ImGuiCol_Button, isSel ? pal.accent : pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_Text, isSel ? pal.bgBase : pal.textPrimary);
        if (ImGui::Button(categories[c])) {
            s_SelectedCategoryIndex = c;
        }
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Render Grid of Palette Item Cards
    ImGui::BeginChild("PaletteItemGrid", ImVec2(0, 0), false);
    
    std::string searchStr = s_PaletteSearch;
    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

    for (const auto& item : s_PaletteItems) {
        // Filter by category
        if (s_SelectedCategoryIndex > 0 && item.category != categories[s_SelectedCategoryIndex]) {
            continue;
        }

        // Filter by search string
        if (!searchStr.empty()) {
            std::string itemNameLower = item.name;
            std::transform(itemNameLower.begin(), itemNameLower.end(), itemNameLower.begin(), ::tolower);
            if (itemNameLower.find(searchStr) == std::string::npos) continue;
        }

        // Card Container
        ImVec2 cardPos = ImGui::GetCursorScreenPos();
        float cardWidth = ImGui::GetContentRegionAvail().x;
        float cardHeight = 58.0f;

        // Reserve layout space in window
        ImGui::Dummy(ImVec2(cardWidth, cardHeight + 6.0f));

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        bool isHovered = ImGui::IsMouseHoveringRect(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight));

        ImU32 bgCol = ImGui::ColorConvertFloat4ToU32(isHovered ? pal.bgElevated : pal.bgHeader);
        ImU32 borderCol = ImGui::ColorConvertFloat4ToU32(isHovered ? pal.accent : pal.borderSubtle);

        drawList->AddRectFilled(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight), bgCol, 4.0f);
        drawList->AddRect(cardPos, ImVec2(cardPos.x + cardWidth, cardPos.y + cardHeight), borderCol, 4.0f);

        // 1. Badge Text on Left (vertically centered)
        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 14.0f, cardPos.y + 18.0f));
        ImGui::TextColored(pal.textSecondary, "%s", item.badgeStr);

        // 2. Title Text on Right Top
        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 80.0f, cardPos.y + 10.0f));
        ImGui::TextColored(pal.textPrimary, "%s", item.name.c_str());

        // 3. Description Text on Right Bottom
        ImGui::SetCursorScreenPos(ImVec2(cardPos.x + 80.0f, cardPos.y + 30.0f));
        ImGui::TextColored(pal.textDisabled, "%s", item.description.c_str());

        // Click to spawn/select item
        if (isHovered && ImGui::IsMouseClicked(0)) {
            SceneNode newNode;
            newNode.name = item.name + "_" + std::to_string(rand() % 1000);
            newNode.type = item.type;
            newNode.meshPath = item.meshPath;

            SceneGraph::Get().AddNode(newNode);

            if (item.type == SceneNodeType::Light && item.defaultLightType >= 0) {
                LightComponent* lc = ComponentRegistry::Get().GetComponent<LightComponent>(newNode.id);
                if (lc) {
                    lc->lightType = item.defaultLightType;
                    if (item.defaultLightType == 0) {
                        lc->intensity = 100000.0f;
                        lc->color[0] = 1.0f; lc->color[1] = 0.95f; lc->color[2] = 0.85f;
                    } else if (item.defaultLightType == 2) {
                        lc->intensity = 5000.0f; lc->range = 25.0f;
                        lc->color[0] = 1.0f; lc->color[1] = 0.90f; lc->color[2] = 0.70f;
                    } else {
                        lc->intensity = 2500.0f; lc->range = 15.0f;
                        lc->color[0] = 1.0f; lc->color[1] = 0.90f; lc->color[2] = 0.70f;
                    }
                }
            }

            EditorState::Get().SetSelection(newNode.name, SceneGraph::GetTypeIconTag(item.type));
            Logger::Get().Info("[Palette] Spawned node '" + newNode.name + "' into SceneGraph.");
        }
    }

    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace EngineEditor
