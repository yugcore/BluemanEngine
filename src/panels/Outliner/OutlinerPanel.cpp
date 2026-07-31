#include "OutlinerPanel.h"
#include "core/SceneGraph.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "widgets/SearchBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace EngineEditor {

static char s_OutlinerSearch[128] = "";
static int s_ActiveScope = 0; // 0=Save, 1=World

static bool CaseInsensitiveContains(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

static bool NodeOrChildMatchesSearch(const SceneNode& node, const std::string& query) {
    if (query.empty()) return true;
    if (CaseInsensitiveContains(node.name, query)) return true;
    for (const auto& child : node.children) {
        if (NodeOrChildMatchesSearch(child, query)) return true;
    }
    return false;
}

static void RenderNodeRow(const SceneNode& node, const std::string& searchQuery) {
    if (!searchQuery.empty() && !NodeOrChildMatchesSearch(node, searchQuery)) {
        return;
    }

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanFullWidth;
    bool isSelected = (EditorState::Get().selectedNodeName == node.name);
    if (isSelected) {
        flags |= ImGuiTreeNodeFlags_Selected;
        const auto& pal = Theme::GetPalette();
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(pal.accentHover.x, pal.accentHover.y, pal.accentHover.z, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(pal.accentActive.x, pal.accentActive.y, pal.accentActive.z, 0.45f));
    }

    if (node.children.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    if (!searchQuery.empty()) {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
    }

    const char* iconTag = SceneGraph::GetTypeIconTag(node.type);
    ImVec4 typeColor = isSelected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : SceneGraph::GetTypeColor(node.type);

    ImGui::PushID(node.name.c_str());

    bool nodeOpen = ImGui::TreeNodeEx("##NodeTree", flags);

    ImGui::SameLine();
    if (iconTag && iconTag[0] != '\0') {
        ImGui::TextColored(typeColor, "%s", iconTag);
        ImGui::SameLine();
    }
    ImGui::TextColored(typeColor, "%s", node.name.c_str());

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        EditorState::Get().SetSelection(node.name, SceneGraph::GetTypeName(node.type));
        Logger::Get().Info("[Outliner] Selected scene node: " + node.name);
    }

    if (isSelected) {
        ImGui::PopStyleColor(3);
    }

    ImGui::TableSetColumnIndex(1);
    ImGui::TextDisabled("%s", node.world.c_str());

    ImGui::TableSetColumnIndex(2);
    ImGui::TextDisabled("%s", node.panel.c_str());

    ImGui::PopID();

    if (nodeOpen && !(flags & ImGuiTreeNodeFlags_Leaf)) {
        for (const auto& child : node.children) {
            RenderNodeRow(child, searchQuery);
        }
        ImGui::TreePop();
    }
}

void RenderOutlinerPanel(bool* pOpen) {
    if (!ImGui::Begin("Outliner", pOpen)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // --- Scope filter buttons ---
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

    auto ScopeTab = [&](const char* label, int idx) {
        bool active = (s_ActiveScope == idx);
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
            ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
            ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
        }
        if (ImGui::Button(label, ImVec2(0, Theme::Metrics::rowHeight))) s_ActiveScope = idx;
        ImGui::PopStyleColor(2);
    };

    ScopeTab("Save", 0);
    ImGui::SameLine();
    ScopeTab("World", 1);

    ImGui::PopStyleVar(2);

    // --- Search bar ---
    ImGui::SameLine(ImGui::GetWindowWidth() - 200.0f);
    Widgets::RenderSearchBar("##OutlinerSearch", s_OutlinerSearch, sizeof(s_OutlinerSearch), "Search scene hierarchy...", 190.0f);
    
    ImGui::Spacing();

    // --- Tree Table ---
    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                 ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_BordersInnerV |
                                 ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("OutlinerTreeTable", 3, tableFlags)) {
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.55f);
        ImGui::TableSetupColumn("World", ImGuiTableColumnFlags_WidthStretch, 0.25f);
        ImGui::TableSetupColumn("Panel", ImGuiTableColumnFlags_WidthStretch, 0.20f);
        ImGui::TableHeadersRow();

        std::string query = s_OutlinerSearch;
        const auto& rootNodes = SceneGraph::Get().GetRootNodes();

        for (const auto& root : rootNodes) {
            RenderNodeRow(root, query);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

} // namespace EngineEditor
