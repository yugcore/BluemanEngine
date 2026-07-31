#include "ProjectExplorerPanel.h"
#include "core/AssetRegistry.h"
#include "widgets/SearchBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>

namespace EngineEditor {

static char s_ExplorerSearch[128] = "";

static void RenderExplorerTreeNode(const AssetFolder& folder) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    if (folder.subfolders.empty()) {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    }

    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
    bool isOpen = ImGui::TreeNodeEx(folder.path.c_str(), flags, "%s", folder.name.c_str());
    ImGui::PopStyleColor();

    if (isOpen && !(flags & ImGuiTreeNodeFlags_Leaf)) {
        for (const auto& sub : folder.subfolders) {
            RenderExplorerTreeNode(sub);
        }
        ImGui::TreePop();
    }
}

void RenderProjectExplorerPanel(bool* pOpen) {
    if (!ImGui::Begin("Project Explorer", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    Widgets::RenderSearchBar("##ExplorerSearch", s_ExplorerSearch, sizeof(s_ExplorerSearch), "Search files...", ImGui::GetContentRegionAvail().x);
    ImGui::Spacing();

    if (Theme::GetFontAtlas().sectionHeaderFont)
        ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.textPrimary, "ZeGFX Workspace");
    if (Theme::GetFontAtlas().sectionHeaderFont)
        ImGui::PopFont();
    
    ImGui::Spacing();

    const auto& rootFolder = AssetRegistry::Get().GetRootFolder();
    RenderExplorerTreeNode(rootFolder);

    ImGui::End();
}

} // namespace EngineEditor
