#include "ContentBrowserPanel.h"
#include "core/AssetRegistry.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "widgets/SearchBar.h"
#include "widgets/AssetTile.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"
#include "third_party/IconsFontAwesome6.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace EngineEditor {

static char s_SearchFilter[128] = "";

static bool CaseInsensitiveContains(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

static void RenderFolderTreeNode(const AssetFolder& folder, std::string& selectedFolder) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (folder.path == selectedFolder) {
        flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (folder.subfolders.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const auto& pal = Theme::GetPalette();
    bool isOpen = false;
    
    ImGui::PushStyleColor(ImGuiCol_Text, (folder.path == selectedFolder) ? pal.textPrimary : pal.textSecondary);
    isOpen = ImGui::TreeNodeEx(folder.path.c_str(), flags, "%s", folder.name.c_str());
    ImGui::PopStyleColor();

    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selectedFolder = folder.path;
        EditorState::Get().selectedFolderPath = folder.path;
    }

    if (isOpen && !(flags & ImGuiTreeNodeFlags_Leaf)) {
        for (const auto& sub : folder.subfolders) {
            RenderFolderTreeNode(sub, selectedFolder);
        }
        ImGui::TreePop();
    }
}

void RenderContentBrowserPanel(bool* pOpen) {
    if (!ImGui::Begin("Content Browser", pOpen)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // 1. Top Control Strip (+ Add / Import / Save All) with styled buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));
    
    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

    if (ImGui::Button("+ Add", ImVec2(55.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] + Add asset dialog opened.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Import", ImVec2(55.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] Import external asset selected.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Save All", ImVec2(55.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] Save All cooked assets completed.");
    }
    ImGui::PopStyleColor(2);

    // Search bar (right-aligned)
    ImGui::SameLine(ImGui::GetWindowWidth() - 220.0f);
    Widgets::RenderSearchBar("##SearchFilter", s_SearchFilter, sizeof(s_SearchFilter), "Search assets...", 210.0f);

    ImGui::PopStyleVar(2);

    ImGui::Spacing();
    
    // Subtle separator
    ImVec2 sepPos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        sepPos, ImVec2(sepPos.x + ImGui::GetContentRegionAvail().x, sepPos.y),
        ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    ImGui::Spacing();

    // 2. Two-Pane Split Layout
    ImGui::Columns(2, "ContentBrowserColumns", true);
    
    static bool setColumnWidth = true;
    if (setColumnWidth) {
        ImGui::SetColumnWidth(0, 200.0f);
        setColumnWidth = false;
    }

    // --- Left Pane: Folder Tree ---
    ImGui::BeginChild("FolderTreeChild", ImVec2(0, 0), false);
    
    // Section header
    if (Theme::GetFontAtlas().sectionHeaderFont)
        ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.textDisabled, "ZeGFX Workspace");
    if (Theme::GetFontAtlas().sectionHeaderFont)
        ImGui::PopFont();
    
    ImGui::Spacing();

    const auto& rootFolder = AssetRegistry::Get().GetRootFolder();
    RenderFolderTreeNode(rootFolder, EditorState::Get().selectedFolderPath);

    ImGui::EndChild();

    ImGui::NextColumn();

    // --- Right Pane: Asset Items ---
    ImGui::BeginChild("AssetItemsChild", ImVec2(0, 0), false);
    
    const AssetFolder* currentFolder = AssetRegistry::Get().FindFolder(EditorState::Get().selectedFolderPath);
    std::string searchQuery = s_SearchFilter;

    if (!searchQuery.empty()) {
        ImGui::TextDisabled("Search Results for: \"%s\"", s_SearchFilter);
    } else if (currentFolder) {
        ImGui::TextDisabled("Location: %s", currentFolder->path.c_str());
    } else {
        ImGui::TextDisabled("Location: Root");
    }
    ImGui::Spacing();

    std::vector<AssetItem> itemsToDisplay;

    auto collectItems = [&](auto& self, const AssetFolder& folder) -> void {
        for (const auto& item : folder.items) {
            if (searchQuery.empty() || CaseInsensitiveContains(item.name, searchQuery) || CaseInsensitiveContains(folder.name, searchQuery)) {
                itemsToDisplay.push_back(item);
            }
        }
        for (const auto& sub : folder.subfolders) {
            self(self, sub);
        }
    };

    if (!searchQuery.empty()) {
        collectItems(collectItems, rootFolder);
    } else if (currentFolder) {
        itemsToDisplay = currentFolder->items;
    }

    if (itemsToDisplay.empty()) {
        ImGui::TextDisabled("No assets found in this folder.");
    } else {
        float itemWidth = 140.0f;
        float itemHeight = 70.0f;
        float windowWidth = ImGui::GetContentRegionAvail().x;
        int columnsCount = (int)(windowWidth / (itemWidth + Theme::Metrics::sectionIndent));
        if (columnsCount < 1) columnsCount = 1;

        int currentCol = 0;
        for (size_t i = 0; i < itemsToDisplay.size(); ++i) {
            const auto& item = itemsToDisplay[i];
            bool isSelected = (EditorState::Get().selectedAssetPath == item.path);

            ImVec4 typeColor = AssetRegistry::GetTypeColor(item.type);
            const char* typeName = AssetRegistry::GetTypeName(item.type);

            std::string tileId = "Tile_" + std::to_string(i);
            if (Widgets::RenderAssetTile(tileId.c_str(), item.name.c_str(), typeName, typeColor, isSelected, itemWidth, itemHeight)) {
                EditorState::Get().SetSelection(item.name, typeName, item.path);
                Logger::Get().Info("[ContentBrowser] Selected asset: " + item.name);
            }

            currentCol++;
            if (currentCol < columnsCount) {
                ImGui::SameLine(0.0f, Theme::Metrics::sectionIndent);
            } else {
                currentCol = 0;
            }
        }
    }

    ImGui::EndChild();

    ImGui::Columns(1);
    ImGui::End();
}

} // namespace EngineEditor
