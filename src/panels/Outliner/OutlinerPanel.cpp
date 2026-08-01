#include "OutlinerPanel.h"
#include "core/SceneGraph.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <cstdio>

namespace EngineEditor {

static char s_OutlinerSearch[128] = "";
static bool s_FilterVisibleOnly = false;
static bool s_FilterPinnedOnly = false;
static std::unordered_map<std::string, bool> s_OpenStates;

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

static uint32_t CountNodes(const std::vector<SceneNode>& nodes) {
    uint32_t count = 0;
    for (const auto& node : nodes) {
        count += 1;
        count += CountNodes(node.children);
    }
    return count;
}

static void RenderNodeRow(const SceneNode& node, const std::string& searchQuery, int depth, float typeColWidth, const Theme::Palette& pal, int rowIndex) {
    if (!searchQuery.empty() && !NodeOrChildMatchesSearch(node, searchQuery)) {
        return;
    }

    const float rowHeight = 22.0f;
    const float indentPerLevel = 18.0f;

    ImGui::PushID(node.name.c_str());

    bool isSelected = (EditorState::Get().selectedNodeName == node.name);
    ImVec2 rMin = ImGui::GetCursorScreenPos();
    float availWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 rMax = ImVec2(rMin.x + availWidth, rMin.y + rowHeight);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Interaction target for full-width row
    ImGui::ItemSize(ImVec2(availWidth, rowHeight));
    ImGui::ItemAdd(ImRect(rMin, rMax), ImGui::GetID(node.name.c_str()));

    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked();

    // 1. Row Background Container (Selection > Hover > Zebra Striping)
    if (isSelected) {
        // Selected: Solid 3px left-edge accent bar + 24% accent full-row background fill
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.24f)));
        dl->AddRectFilled(rMin, ImVec2(rMin.x + 3.0f, rMax.y), ImGui::ColorConvertFloat4ToU32(pal.accent));
    } else if (hovered) {
        // Hovered: Full-width row highlight
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(pal.bgElevated));
    } else if (rowIndex % 2 == 1) {
        // Odd row zebra stripe: subtle 3-4% background tint
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(ImVec4(pal.bgHeader.x, pal.bgHeader.y, pal.bgHeader.z, 0.35f)));
    }

    if (clicked) {
        EditorState::Get().SetSelection(node.name, SceneGraph::GetTypeName(node.type));
        Logger::Get().Info("[Outliner] Selected node: " + node.name);
    }

    // 2. Persistent Vertical Guidelines connecting parent rows to children
    for (int d = 1; d <= depth; ++d) {
        float guideX = rMin.x + (d * indentPerLevel) - 8.0f;
        dl->AddLine(ImVec2(guideX, rMin.y), ImVec2(guideX, rMax.y), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    }

    float currentX = rMin.x + (depth * indentPerLevel) + 4.0f;

    // 3. Caret for Expand/Collapse (Nodes with children)
    bool hasChildren = !node.children.empty();
    if (s_OpenStates.find(node.name) == s_OpenStates.end()) {
        s_OpenStates[node.name] = true;
    }
    if (!searchQuery.empty()) {
        s_OpenStates[node.name] = true;
    }

    bool isOpen = s_OpenStates[node.name];

    if (hasChildren) {
        ImU32 caretColor = ImGui::ColorConvertFloat4ToU32(pal.textSecondary);
        float centerY = rMin.y + rowHeight * 0.5f;

        if (isOpen) {
            // Down-pointing vector triangle caret
            ImVec2 p1(currentX + 2.0f, centerY - 2.0f);
            ImVec2 p2(currentX + 10.0f, centerY - 2.0f);
            ImVec2 p3(currentX + 6.0f, centerY + 3.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretColor);
        } else {
            // Right-pointing vector triangle caret
            ImVec2 p1(currentX + 3.0f, centerY - 4.0f);
            ImVec2 p2(currentX + 8.0f, centerY);
            ImVec2 p3(currentX + 3.0f, centerY + 4.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretColor);
        }

        ImRect caretRect(ImVec2(currentX - 2.0f, rMin.y), ImVec2(currentX + 14.0f, rMax.y));
        if (ImGui::IsMouseClicked(0) && caretRect.Contains(ImGui::GetMousePos())) {
            s_OpenStates[node.name] = !isOpen;
            isOpen = s_OpenStates[node.name];
        }
    }
    currentX += 14.0f;

    // 4. Fixed-size 16x16 Icon Bounding Box & Vector/Glyph Icon Rendering
    float iconBoxSize = 16.0f;
    ImVec2 iconBoxMin = ImVec2(currentX, rMin.y + (rowHeight - iconBoxSize) * 0.5f);
    ImVec2 iconBoxMax = ImVec2(iconBoxMin.x + iconBoxSize, iconBoxMin.y + iconBoxSize);

    ImVec4 iconColor = SceneGraph::GetTypeColor(node.type);

    if (node.type == SceneNodeType::Folder) {
        // Folder Icon Shape (Gold/Amber)
        ImU32 folderCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.75f, 0.35f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 3.0f), ImVec2(iconBoxMax.x - 1.0f, iconBoxMax.y - 1.0f), folderCol, 2.0f);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 1.0f), ImVec2(iconBoxMin.x + 7.0f, iconBoxMin.y + 4.0f), folderCol, 1.0f);
    } else if (node.type == SceneNodeType::Light) {
        // Light Icon (Amber Yellow Circle)
        ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.96f, 0.82f, 0.28f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.5f, lightCol);
    } else if (node.type == SceneNodeType::Camera) {
        // Camera Icon (Green Rect)
        ImU32 camCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.85f, 0.50f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 4.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), camCol, 2.0f);
    } else if (node.type == SceneNodeType::Audio) {
        // Audio Icon (Purple Circle)
        ImU32 audioCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.85f, 0.50f, 0.90f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.0f, audioCol);
    } else {
        // Mesh / Actor / Default (Cyan / Blue Rounded Box)
        ImU32 meshCol = ImGui::ColorConvertFloat4ToU32(iconColor);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 2.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), meshCol, 2.0f);
    }

    currentX += iconBoxSize + 6.0f; // Fixed spacing after 16x16 icon box

    // 5. Item Name (Text aligned to exact left edge)
    ImVec4 nameColor = isSelected ? pal.textPrimary : pal.textPrimary;
    ImVec2 namePos = ImVec2(currentX, rMin.y + (rowHeight - ImGui::GetTextLineHeight()) * 0.5f);

    float maxNameX = rMax.x - typeColWidth - 10.0f;
    dl->PushClipRect(rMin, ImVec2(maxNameX, rMax.y), true);
    dl->AddText(namePos, ImGui::ColorConvertFloat4ToU32(nameColor), node.name.c_str());
    dl->PopClipRect();

    // 6. Right-Aligned Type Column
    const char* typeName = SceneGraph::GetTypeName(node.type);
    float typeX = rMax.x - typeColWidth + 8.0f;
    ImVec2 typePos = ImVec2(typeX, rMin.y + (rowHeight - ImGui::GetTextLineHeight()) * 0.5f);
    dl->AddText(typePos, ImGui::ColorConvertFloat4ToU32(pal.textDisabled), typeName);

    ImGui::PopID();

    // --- Render Children Recursively ---
    if (hasChildren && isOpen) {
        int childIdx = 0;
        for (const auto& child : node.children) {
            RenderNodeRow(child, searchQuery, depth + 1, typeColWidth, pal, childIdx++);
        }
    }
}

void RenderOutlinerPanel(bool* pOpen) {
    if (!ImGui::Begin("Outliner", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // ====================================================================
    // ROW 1: SEARCH INPUT (Full Width with Inline Filter & Chevron Icons)
    // ====================================================================
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, pal.bgElevated);

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##OutlinerSearch", "Search scene hierarchy...", s_OutlinerSearch, sizeof(s_OutlinerSearch));

    ImVec2 sMin = ImGui::GetItemRectMin();
    ImVec2 sMax = ImGui::GetItemRectMax();
    float searchCenterY = (sMin.y + sMax.y) * 0.5f - ImGui::GetTextLineHeight() * 0.5f;

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Left inline filter/funnel icon inside field (solid dot indicator)
    dl->AddCircleFilled(ImVec2(sMin.x + 12.0f, (sMin.y + sMax.y) * 0.5f), 3.5f, ImGui::ColorConvertFloat4ToU32(pal.textSecondary));

    // Right inline dropdown chevron icon inside field (down-pointing vector triangle)
    float chevronX = sMax.x - 16.0f;
    float cY = (sMin.y + sMax.y) * 0.5f;
    ImU32 chevCol = ImGui::ColorConvertFloat4ToU32(pal.textSecondary);
    dl->AddTriangleFilled(
        ImVec2(chevronX, cY - 2.0f),
        ImVec2(chevronX + 8.0f, cY - 2.0f),
        ImVec2(chevronX + 4.0f, cY + 3.0f), chevCol);

    // Click area for dropdown options
    ImRect chevronRect(ImVec2(sMax.x - 24.0f, sMin.y), sMax);
    if (ImGui::IsMouseClicked(0) && chevronRect.Contains(ImGui::GetMousePos())) {
        ImGui::OpenPopup("SearchTypeOptionsPopup");
    }

    if (ImGui::BeginPopup("SearchTypeOptionsPopup")) {
        ImGui::MenuItem("Search by Name", nullptr, true);
        ImGui::MenuItem("Search by Type");
        ImGui::MenuItem("Search by Component");
        ImGui::EndPopup();
    }

    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    ImGui::Spacing();

    // ====================================================================
    // ROW 2: TOOLBAR ROW & COLUMN HEADERS
    // ====================================================================
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 2.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f, 0.0f));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

    // Toggle Eye (Visibility)
    ImGui::PushStyleColor(ImGuiCol_Text, s_FilterVisibleOnly ? pal.accent : pal.textSecondary);
    if (ImGui::Button("\xE2\x97\x8F##VisToggle", ImVec2(20.0f, 20.0f))) {
        s_FilterVisibleOnly = !s_FilterVisibleOnly;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Visibility Filter");
    ImGui::PopStyleColor();

    ImGui::SameLine();

    // Toggle Pin (Favorite)
    ImGui::PushStyleColor(ImGuiCol_Text, s_FilterPinnedOnly ? pal.accent : pal.textSecondary);
    if (ImGui::Button("\xE2\x98\x85##PinToggle", ImVec2(20.0f, 20.0f))) {
        s_FilterPinnedOnly = !s_FilterPinnedOnly;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Pinned Items");
    ImGui::PopStyleColor();

    ImGui::PopStyleColor(2);

    // 1px Vertical divider line after toolbar icons
    ImGui::SameLine(0.0f, 6.0f);
    ImVec2 divP = ImGui::GetCursorScreenPos();
    float divY1 = divP.y + 3.0f;
    float divY2 = divY1 + 14.0f;
    dl->AddLine(ImVec2(divP.x, divY1), ImVec2(divP.x, divY2), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    ImGui::Dummy(ImVec2(1.0f, 0.0f));
    ImGui::SameLine(0.0f, 6.0f);

    // Header Band Background & Semibold Column Headers
    ImVec2 hMin = ImGui::GetCursorScreenPos();
    float hWidth = ImGui::GetContentRegionAvail().x;
    dl->AddRectFilled(ImVec2(hMin.x - 4.0f, hMin.y - 2.0f), ImVec2(hMin.x + hWidth + 8.0f, hMin.y + 22.0f), ImGui::ColorConvertFloat4ToU32(pal.bgHeader));

    float typeColWidth = 140.0f;
    float headerY = ImGui::GetCursorPosY() + (20.0f - ImGui::GetTextLineHeight()) * 0.5f;

    ImGui::SetCursorPosY(headerY);
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
    ImGui::TextColored(pal.textPrimary, "Item Label");

    float availW = ImGui::GetWindowWidth();
    ImGui::SameLine(availW - typeColWidth);
    ImGui::TextColored(pal.textPrimary, "Type");
    if (Theme::GetFontAtlas().sectionHeaderFont) ImGui::PopFont();

    ImGui::PopStyleVar(2);

    ImGui::Spacing();

    // ====================================================================
    // ROW 3: SCROLLABLE TREE ROWS
    // ====================================================================
    const float footerHeight = 24.0f;
    float treeChildHeight = ImGui::GetContentRegionAvail().y - footerHeight;
    if (treeChildHeight < 50.0f) treeChildHeight = 50.0f;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

    if (ImGui::BeginChild("##OutlinerTreeChild", ImVec2(0.0f, treeChildHeight), false, ImGuiWindowFlags_NoScrollbar)) {
        std::string query = s_OutlinerSearch;
        const auto& rootNodes = SceneGraph::Get().GetRootNodes();

        if (rootNodes.empty()) {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f);
            ImGui::SetCursorPosX(16.0f);
            ImGui::TextColored(pal.textDisabled, "No actors in scene.");
            ImGui::SetCursorPosX(16.0f);
            ImGui::TextColored(pal.textSecondary, "Use Object Palette or Create menu to add nodes.");
        } else {
            int rootIdx = 0;
            for (const auto& root : rootNodes) {
                RenderNodeRow(root, query, 0, typeColWidth, pal, rootIdx++);
            }
        }

        // Hotkey shortcuts for Outliner: Del (Delete), Ctrl+D (Duplicate)
        if (ImGui::IsWindowFocused() && !EditorState::Get().selectedNodeName.empty()) {
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                SceneGraph::Get().RemoveNode(EditorState::Get().selectedNodeName);
                Logger::Get().Info("[Outliner] Deleted actor via Del key: " + EditorState::Get().selectedNodeName);
                EditorState::Get().selectedNodeName = "";
            }
            if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) {
                SceneNode* dup = SceneGraph::Get().DuplicateNode(EditorState::Get().selectedNodeName);
                if (dup) {
                    EditorState::Get().SetSelection(dup->name, SceneGraph::GetTypeName(dup->type));
                    Logger::Get().Info("[Outliner] Duplicated actor via Ctrl+D: " + dup->name);
                }
            }
        }

        // Right-Click Context Menu for Outliner
        if (ImGui::BeginPopupContextWindow("OutlinerContextMenu")) {
            if (ImGui::BeginMenu("+ Add Actor")) {
                if (ImGui::MenuItem("Directional Sun Light")) {
                    SceneNode sunNode;
                    sunNode.name = "SunLight_" + std::to_string(rand() % 1000);
                    sunNode.type = SceneNodeType::Light;
                    SceneGraph::Get().AddNode(sunNode);
                    EditorState::Get().SetSelection(sunNode.name, "Light");
                    Logger::Get().Info("[Outliner] Added Directional Sun Light actor");
                }
                if (ImGui::MenuItem("Point Light")) {
                    SceneNode ptNode;
                    ptNode.name = "PointLight_" + std::to_string(rand() % 1000);
                    ptNode.type = SceneNodeType::Light;
                    SceneGraph::Get().AddNode(ptNode);
                    EditorState::Get().SetSelection(ptNode.name, "Light");
                    Logger::Get().Info("[Outliner] Added Point Light actor");
                }
                if (ImGui::MenuItem("Spot Light")) {
                    SceneNode spotNode;
                    spotNode.name = "SpotLight_" + std::to_string(rand() % 1000);
                    spotNode.type = SceneNodeType::Light;
                    SceneGraph::Get().AddNode(spotNode);
                    EditorState::Get().SetSelection(spotNode.name, "Light");
                    Logger::Get().Info("[Outliner] Added Spot Light actor");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("ZePhysics RigidBody Actor")) {
                    SceneNode physNode;
                    physNode.name = "RigidBody_" + std::to_string(rand() % 1000);
                    physNode.type = SceneNodeType::Actor;
                    SceneGraph::Get().AddNode(physNode);
                    EditorState::Get().SetSelection(physNode.name, "Actor");
                    Logger::Get().Info("[Outliner] Added ZePhysics RigidBody actor");
                }
                if (ImGui::MenuItem("Static Mesh Actor")) {
                    SceneNode meshNode;
                    meshNode.name = "StaticMesh_" + std::to_string(rand() % 1000);
                    meshNode.type = SceneNodeType::Actor;
                    SceneGraph::Get().AddNode(meshNode);
                    EditorState::Get().SetSelection(meshNode.name, "Actor");
                    Logger::Get().Info("[Outliner] Added Static Mesh actor");
                }
                if (ImGui::MenuItem("Folder")) {
                    SceneNode folderNode;
                    folderNode.name = "NewFolder_" + std::to_string(rand() % 1000);
                    folderNode.type = SceneNodeType::Folder;
                    SceneGraph::Get().AddNode(folderNode);
                    EditorState::Get().SetSelection(folderNode.name, "Folder");
                    Logger::Get().Info("[Outliner] Added Folder node");
                }
                ImGui::EndMenu();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Duplicate Selected Actor", "Ctrl+D", false, !EditorState::Get().selectedNodeName.empty())) {
                SceneNode* dup = SceneGraph::Get().DuplicateNode(EditorState::Get().selectedNodeName);
                if (dup) {
                    EditorState::Get().SetSelection(dup->name, SceneGraph::GetTypeName(dup->type));
                    Logger::Get().Info("[Outliner] Duplicated actor: " + dup->name);
                }
            }
            if (ImGui::MenuItem("Delete Selected Actor", "Del", false, !EditorState::Get().selectedNodeName.empty())) {
                SceneGraph::Get().RemoveNode(EditorState::Get().selectedNodeName);
                Logger::Get().Info("[Outliner] Deleted actor: " + EditorState::Get().selectedNodeName);
                EditorState::Get().selectedNodeName = "";
            }
            ImGui::EndPopup();
        }
    }
    ImGui::EndChild();

    ImGui::PopStyleVar(2);

    // ====================================================================
    // ROW 4: PINNED FOOTER (Live Actor Count)
    // ====================================================================
    ImVec2 fMin = ImGui::GetCursorScreenPos();
    float fWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 fMax = ImVec2(fMin.x + fWidth, fMin.y + footerHeight);

    // Thin top border line separating tree from footer
    dl->AddLine(fMin, ImVec2(fMax.x, fMin.y), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);

    uint32_t totalActors = CountNodes(SceneGraph::Get().GetRootNodes());
    char footerBuf[64];
    snprintf(footerBuf, sizeof(footerBuf), "%u actors (%u loaded)", totalActors, totalActors);

    float textCenterY = fMin.y + (footerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::SetCursorPosY(textCenterY - ImGui::GetWindowPos().y);
    ImGui::SetCursorPosX(8.0f);
    ImGui::TextColored(pal.textDisabled, "%s", footerBuf);

    ImGui::End();
}

} // namespace EngineEditor
