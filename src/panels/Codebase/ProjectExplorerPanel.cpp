#include "ProjectExplorerPanel.h"
#include "CodeEditorPanel.h"
#include "CodeHighlighter.h"
#include "core/EditorState.h"
#include "widgets/SearchBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>
#include <string>
#include <vector>

namespace EngineEditor {

static char s_ExplorerSearch[128] = "";
static char s_NewFileName[128] = "new_script.zl";
static int s_SelectedLangIndex = 0; // 0: Zelyn (.zl), 1: Zelyn Module (.zyn), 2: C++ (.cpp), 3: Header (.h), 4: Lua (.lua)

struct FileNode {
    std::string name;
    std::string path;
    bool isFolder = false;
    LanguageType lang = LanguageType::Unknown;
    std::vector<FileNode> children;
};

static std::vector<FileNode> GetProjectTree() {
    return {
        { "src", "src", true, LanguageType::Unknown, {
            { "Main.cpp", "src/Main.cpp", false, LanguageType::Cpp, {} },
            { "Renderer.cpp", "src/Renderer.cpp", false, LanguageType::Cpp, {} },
            { "Engine.h", "src/Engine.h", false, LanguageType::Header, {} }
        }},
        { "scripts", "scripts", true, LanguageType::Unknown, {
            { "game_logic.zl", "scripts/game_logic.zl", false, LanguageType::Zelyn, {} },
            { "player_controller.zyn", "scripts/player_controller.zyn", false, LanguageType::Zelyn, {} },
            { "level_loader.zl", "scripts/level_loader.zl", false, LanguageType::Zelyn, {} }
        }},
        { "shaders", "shaders", true, LanguageType::Unknown, {
            { "RayTracingPipeline.hlsl", "shaders/RayTracingPipeline.hlsl", false, LanguageType::Header, {} }
        }},
        { "CMakeLists.txt", "CMakeLists.txt", false, LanguageType::Header, {} }
    };
}

static void RenderFileTreeNode(const FileNode& node, float indentX = 0.0f) {
    const auto& pal = Theme::GetPalette();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

    if (node.isFolder) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
        
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.06f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 1.0f, 0.10f));

        ImVec2 nodePos = ImGui::GetCursorScreenPos();
        ImGuiID nodeID = ImGui::GetID((void*)&node);
        bool isOpenState = ImGui::GetStateStorage()->GetBool(nodeID, false);
        const char* arrowSymbol = isOpenState ? "v" : ">";

        bool isOpen = ImGui::TreeNodeEx((void*)&node, flags, "%s  %s", arrowSymbol, node.name.c_str());

        ImGui::PopStyleColor(3);

        if (isOpen) {
            // Draw thin vertical tree indentation guide line
            ImVec2 childStartPos = ImGui::GetCursorScreenPos();
            float guideX = nodePos.x + 12.0f;
            
            for (size_t i = 0; i < node.children.size(); ++i) {
                RenderFileTreeNode(node.children[i], guideX);
            }

            ImVec2 childEndPos = ImGui::GetCursorScreenPos();
            drawList->AddLine(
                ImVec2(guideX, nodePos.y + 20.0f),
                ImVec2(guideX, childEndPos.y - 8.0f),
                ImGui::ColorConvertFloat4ToU32(ImVec4(pal.borderSubtle.x, pal.borderSubtle.y, pal.borderSubtle.z, 0.4f)),
                1.0f
            );

            ImGui::TreePop();
        }
    } else {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
        bool isSelected = (EditorState::Get().activeCodeFileName == node.name);
        if (isSelected) flags |= ImGuiTreeNodeFlags_Selected;

        // System Accent color for selection state
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.18f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.35f));
        ImGui::PushStyleColor(ImGuiCol_Text, isSelected ? pal.textPrimary : pal.textSecondary);

        bool nodeClicked = ImGui::TreeNodeEx((void*)&node, flags, "   %s", node.name.c_str());

        ImGui::PopStyleColor(4);

        // Single-click on a file opens it in the Code Editor (Priority 0)
        if (ImGui::IsItemClicked(0)) {
            OpenCodeDocument(node.name);
        }
    }

    ImGui::PopStyleVar();
}

void RenderProjectExplorerPanel(bool* pOpen) {
    if (!ImGui::Begin("Project Explorer", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();
    auto& state = EditorState::Get();

    // Padding Container for Top Bar & Tree
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    // Top Action Bar: Search Bar + Prominent "+ Add File" Button
    float availWidth = ImGui::GetContentRegionAvail().x;
    float btnWidth = 72.0f;
    float searchWidth = availWidth - btnWidth - 8.0f;

    Widgets::RenderSearchBar("##ExplorerSearch", s_ExplorerSearch, sizeof(s_ExplorerSearch), "Search files...", searchWidth);
    ImGui::SameLine(0.0f, 8.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(pal.accent.x * 1.1f, pal.accent.y * 1.1f, pal.accent.z * 1.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
    if (ImGui::Button("+ Add", ImVec2(btnWidth, 0.0f))) {
        state.showNewFileDialog = true;
    }
    ImGui::PopStyleColor(4);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Root Workspace Card Header
    ImVec2 headerMin = ImGui::GetCursorScreenPos();
    float headerWidth = ImGui::GetContentRegionAvail().x;
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(
        headerMin,
        ImVec2(headerMin.x + headerWidth, headerMin.y + 28.0f),
        ImGui::ColorConvertFloat4ToU32(pal.bgElevated), 4.0f
    );
    drawList->AddRect(
        headerMin,
        ImVec2(headerMin.x + headerWidth, headerMin.y + 28.0f),
        ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 4.0f
    );

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);

    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.textPrimary, "ZeGFX Project Workspace");
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PopFont();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);

    // Full-Width File Tree Rendering
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16.0f);
    static auto tree = GetProjectTree();
    for (const auto& root : tree) {
        RenderFileTreeNode(root);
    }
    ImGui::PopStyleVar();

    ImGui::PopStyleVar(); // FramePadding

    // Modal New File Dialog with Language Selection
    if (state.showNewFileDialog) {
        ImGui::OpenPopup("Create New File / Script");
    }

    if (ImGui::BeginPopupModal("Create New File / Script", &state.showNewFileDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextColored(pal.textPrimary, "Select Target Language:");
        ImGui::Spacing();

        // Language Selector Buttons with Neutral Monochrome Styling
        const struct { const char* name; const char* badge; const char* ext; } langs[] = {
            { "Zelyn Script", "", ".zl" },
            { "Zelyn Module", "", ".zyn" },
            { "C++ Source", "", ".cpp" },
            { "C++ Header", "", ".h" },
            { "Lua Script", "", ".lua" }
        };

        for (int i = 0; i < 5; ++i) {
            bool selected = (s_SelectedLangIndex == i);
            ImGui::PushStyleColor(ImGuiCol_Button, selected ? pal.accent : pal.bgElevated);
            ImGui::PushStyleColor(ImGuiCol_Text, selected ? ImVec4(1, 1, 1, 1) : pal.textPrimary);
            std::string btnText = std::string(langs[i].badge) + " " + langs[i].name;
            if (ImGui::Button(btnText.c_str(), ImVec2(130.0f, 28.0f))) {
                s_SelectedLangIndex = i;
                std::string stem = "new_script";
                s_NewFileName[0] = '\0';
                snprintf(s_NewFileName, sizeof(s_NewFileName), "%s%s", stem.c_str(), langs[i].ext);
            }
            ImGui::PopStyleColor(2);
            if (i < 4) ImGui::SameLine(0.0f, 6.0f);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("File Name:");
        ImGui::InputText("##NewFileName", s_NewFileName, sizeof(s_NewFileName));

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
        if (ImGui::Button("Create & Open", ImVec2(120.0f, 28.0f))) {
            std::string fname = s_NewFileName;
            OpenCodeDocument(fname);
            state.showNewFileDialog = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();

        ImGui::SameLine(0.0f, 12.0f);
        if (ImGui::Button("Cancel", ImVec2(90.0f, 28.0f))) {
            state.showNewFileDialog = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

} // namespace EngineEditor
