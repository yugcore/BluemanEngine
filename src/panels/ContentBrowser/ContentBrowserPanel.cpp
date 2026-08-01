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

void RenderContentBrowserPanel(bool* pOpen) {
    bool* openPtr = pOpen ? pOpen : &EditorState::Get().settings.showContentBrowser;
    if (!*openPtr) return;

    if (!ImGui::Begin("Content Browser", openPtr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // 1. Top Control Strip (+ Add / Import / Save All, [<] [>], Breadcrumbs, Search, Dock in Layout, Settings)
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(12.0f, 6.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(Theme::Metrics::intraGroupGap, 4.0f));
    
    // + Add (Vibrant primary action button)
    ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.accentHover);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
    if (ImGui::Button("+ Add", ImVec2(0.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] + Add asset dialog opened.");
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

    if (ImGui::Button("Import", ImVec2(0.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] Import external asset selected.");
    }
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (ImGui::Button("Save All", ImVec2(0.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] Save All cooked assets completed.");
    }

    // Navigation history buttons [<] [>]
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    ImGui::Button("<", ImVec2(28.0f, Theme::Metrics::rowHeight));
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::Button(">", ImVec2(28.0f, Theme::Metrics::rowHeight));

    // Path Breadcrumbs (All > Content > StarterContent > Materials)
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    ImGui::AlignTextToFramePadding();

    const AssetFolder* currentFolder = AssetRegistry::Get().FindFolder(EditorState::Get().selectedFolderPath);
    if (!currentFolder) {
        currentFolder = AssetRegistry::Get().FindFolder("ZeGFX Workspace/Content/StarterContent/Materials");
    }

    ImGui::TextColored(pal.textDisabled, "All  >");
    ImGui::SameLine(0.0f, 6.0f);
    if (currentFolder) {
        ImGui::TextColored(pal.textPrimary, "%s", currentFolder->path.c_str());
    } else {
        ImGui::TextColored(pal.textPrimary, "ZeGFX Workspace/Content/StarterContent/Materials");
    }

    // Search Bar right after breadcrumbs
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    Widgets::RenderSearchBar("##SearchFilter", s_SearchFilter, sizeof(s_SearchFilter), "Search Materials", 240.0f);

    // Right-aligned header tools: Dock in Layout & Settings
    float rightToolsWidth = 220.0f;
    float rightStart = ImGui::GetWindowWidth() - rightToolsWidth - Theme::Metrics::panelLeftMargin;
    if (rightStart > ImGui::GetCursorPosX()) {
        ImGui::SameLine(rightStart);
    } else {
        ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    }

    if (ImGui::Button("Dock in Layout", ImVec2(0.0f, Theme::Metrics::rowHeight))) {
        Logger::Get().Info("[ContentBrowser] Docked in layout.");
    }
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    if (ImGui::Button("Settings", ImVec2(0.0f, Theme::Metrics::rowHeight))) {
        ImGui::OpenPopup("ContentBrowserSettingsPopup");
    }
    if (ImGui::BeginPopup("ContentBrowserSettingsPopup")) {
        ImGui::MenuItem("Show Engine Content");
        ImGui::MenuItem("Show C++ Classes");
        ImGui::MenuItem("Thumbnail Size: Medium");
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(2);

    ImGui::Spacing();

    // 2. Horizontal Folder Navigation Bar (Favorites & Folders below control strip)
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 3.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 0.0f));

    static int s_ActiveTypeFilter = 0; // 0: All, 1: Material, 2: Mesh, 3: Texture, 4: Script, 5: Audio, 6: Level, 7: VFX
    static int s_SortMode = 0;         // 0: Name (A-Z), 1: Name (Z-A), 2: Type

    // Category Type Filter Bar
    ImGui::TextDisabled("Filter:");
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);

    const char* filterLabels[] = { "All", "Materials", "Meshes", "Textures", "Scripts", "Audio", "Levels", "VFX" };
    for (int f = 0; f < 8; ++f) {
        if (f > 0) ImGui::SameLine(0.0f, 4.0f);
        bool isSel = (s_ActiveTypeFilter == f);
        ImGui::PushStyleColor(ImGuiCol_Button, isSel ? pal.accent : pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_Text, isSel ? pal.bgBase : pal.textPrimary);
        if (ImGui::Button(filterLabels[f])) {
            s_ActiveTypeFilter = f;
        }
        ImGui::PopStyleColor(2);
    }

    // Sort Dropdown Combo on the right side
    ImGui::SameLine(0.0f, Theme::Metrics::groupGap);
    ImGui::TextDisabled("Sort:");
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::SetNextItemWidth(120.0f);
    const char* sortModes[] = { "Name (A-Z)", "Name (Z-A)", "Type" };
    ImGui::Combo("##SortCombo", &s_SortMode, sortModes, 3);

    ImGui::PopStyleVar(3);
    ImGui::Spacing();
    
    // Subtle separator line
    ImVec2 sepPos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddLine(
        sepPos, ImVec2(sepPos.x + ImGui::GetContentRegionAvail().x, sepPos.y),
        ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    ImGui::Spacing();

    // 3. Full-Width Landscape Asset Browser Grid (BELOW the folder bar)
    ImGui::BeginChild("FullLandscapeAssetGrid", ImVec2(0, 0), false);

    std::string searchQuery = s_SearchFilter;
    std::vector<AssetItem> itemsToDisplay;

    const auto& rootFolder = AssetRegistry::Get().GetRootFolder();
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

    // Filter items by active category filter
    if (s_ActiveTypeFilter > 0) {
        AssetItemType targetType = AssetItemType::Unknown;
        if (s_ActiveTypeFilter == 1) targetType = AssetItemType::Material;
        else if (s_ActiveTypeFilter == 2) targetType = AssetItemType::Mesh;
        else if (s_ActiveTypeFilter == 3) targetType = AssetItemType::Texture;
        else if (s_ActiveTypeFilter == 4) targetType = AssetItemType::Script;
        else if (s_ActiveTypeFilter == 5) targetType = AssetItemType::Audio;
        else if (s_ActiveTypeFilter == 6) targetType = AssetItemType::Level;
        else if (s_ActiveTypeFilter == 7) targetType = AssetItemType::VFX;

        std::vector<AssetItem> filtered;
        for (const auto& item : itemsToDisplay) {
            if (item.type == targetType) filtered.push_back(item);
        }
        itemsToDisplay = std::move(filtered);
    }

    // Sort items
    if (s_SortMode == 0) {
        std::sort(itemsToDisplay.begin(), itemsToDisplay.end(), [](const AssetItem& a, const AssetItem& b) {
            return a.name < b.name;
        });
    } else if (s_SortMode == 1) {
        std::sort(itemsToDisplay.begin(), itemsToDisplay.end(), [](const AssetItem& a, const AssetItem& b) {
            return a.name > b.name;
        });
    } else if (s_SortMode == 2) {
        std::sort(itemsToDisplay.begin(), itemsToDisplay.end(), [](const AssetItem& a, const AssetItem& b) {
            return static_cast<int>(a.type) < static_cast<int>(b.type);
        });
    }

    if (itemsToDisplay.empty()) {
        ImGui::TextDisabled("No assets found matching the selected criteria.");
    } else {
        float itemWidth = Theme::Metrics::tileWidth;   // 124.0f
        float itemHeight = 128.0f;                      // Generous vertical headroom for text
        float cellGap = Theme::Metrics::tileGap;       // 16.0f
        float windowWidth = ImGui::GetContentRegionAvail().x;
        int columnsCount = (int)((windowWidth + cellGap) / (itemWidth + cellGap));
        if (columnsCount < 1) columnsCount = 1;

        int currentCol = 0;
        for (size_t i = 0; i < itemsToDisplay.size(); ++i) {
            const auto& item = itemsToDisplay[i];
            bool isSelected = (EditorState::Get().selectedAssetPath == item.path);

            ImVec4 typeColor = AssetRegistry::GetTypeColor(item.type);
            const char* typeName = AssetRegistry::GetTypeName(item.type);

            std::string tileId = "Tile_" + std::to_string(i);
            if (Widgets::RenderAssetTile(tileId.c_str(), item.name.c_str(), item.type, typeName, typeColor, isSelected, itemWidth, itemHeight)) {
                EditorState::Get().SetSelection(item.name, typeName, item.path);
                Logger::Get().Info("[ContentBrowser] Selected asset: " + item.name);
            }

            currentCol++;
            if (currentCol < columnsCount) {
                ImGui::SameLine(0.0f, cellGap);
            } else {
                currentCol = 0;
                ImGui::Dummy(ImVec2(0.0f, cellGap));
            }
        }

        // Bottom Item Count Footer
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::TextDisabled("%zu items", itemsToDisplay.size());
    }

    ImGui::EndChild();
    ImGui::End();
}

} // namespace EngineEditor
