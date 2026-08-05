#include "OutlinerPanel.h"
#include "engine/scene/SceneGraph.h"
#include "core/EditorState.h"
#include "engine/core/Logger.h"
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

static ImVec4 GetSceneNodeTypeColor(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // Light Gray
        case SceneNodeType::Actor:         return ImVec4(0.30f, 0.75f, 0.95f, 1.00f); // Cyan
        case SceneNodeType::Light:         return ImVec4(0.95f, 0.80f, 0.25f, 1.00f); // Amber Yellow
        case SceneNodeType::Camera:        return ImVec4(0.40f, 0.85f, 0.50f, 1.00f); // Green
        case SceneNodeType::Audio:         return ImVec4(0.90f, 0.45f, 0.25f, 1.00f); // Orange
        case SceneNodeType::SkyAtmosphere: return ImVec4(0.70f, 0.45f, 0.95f, 1.00f); // Purple
        case SceneNodeType::VolumetricFog: return ImVec4(0.55f, 0.70f, 0.90f, 1.00f); // Mist Blue
        case SceneNodeType::Component:     return ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // Muted
        case SceneNodeType::Terrain:       return ImVec4(0.45f, 0.75f, 0.35f, 1.00f); // Forest Green
        case SceneNodeType::FoliageCluster:return ImVec4(0.35f, 0.85f, 0.45f, 1.00f); // Emerald
        case SceneNodeType::PathPoint:     return ImVec4(0.85f, 0.65f, 0.35f, 1.00f); // Soil Brown
        default:                           return ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    }
}

static char s_OutlinerSearch[128] = "";
static bool s_FilterVisibleOnly = false;
static bool s_FilterPinnedOnly = false;
static std::unordered_map<std::string, bool> s_OpenStates;

static uint64_t s_RenamingNodeId = 0;
static char s_RenameBuf[128] = "";

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

static void CountNodesRecursive(const std::vector<SceneNode>& nodes, uint32_t& total, uint32_t& visibleCount) {
    for (const auto& node : nodes) {
        total += 1;
        if (node.visible) visibleCount += 1;
        CountNodesRecursive(node.children, total, visibleCount);
    }
}

static void RenderNodeRow(const SceneNode& node, const std::string& searchQuery, int depth, float typeColWidth, const Theme::Palette& pal, int rowIndex) {
    if (!searchQuery.empty() && !NodeOrChildMatchesSearch(node, searchQuery)) {
        return;
    }
    if (s_FilterVisibleOnly && !node.visible) {
        return;
    }

    const float rowHeight = 22.0f;
    const float indentPerLevel = 18.0f;

    ImGui::PushID((int)node.id);

    bool isSelected = (EditorState::Get().selectedNodeName == node.name);
    ImVec2 rMin = ImGui::GetCursorScreenPos();
    float availWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 rMax = ImVec2(rMin.x + availWidth, rMin.y + rowHeight);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Interaction target for full-width row
    ImGui::ItemSize(ImVec2(availWidth, rowHeight));
    ImGui::ItemAdd(ImRect(rMin, rMax), ImGui::GetID(node.name.c_str()));

    bool hovered = ImGui::IsItemHovered();
    bool clicked = ImGui::IsItemClicked(0);
    bool doubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0);

    // Drag and Drop Source
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        uint64_t dragId = node.id;
        ImGui::SetDragDropPayload("OUTLINER_NODE", &dragId, sizeof(uint64_t));
        ImGui::Text("Move node: %s", node.name.c_str());
        ImGui::EndDragDropSource();
    }

    // Drag and Drop Target (Parenting)
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_NODE")) {
            uint64_t sourceId = *(const uint64_t*)payload->Data;
            SceneGraph::Get().ReparentNode(sourceId, node.id);
            Logger::Get().Info("[Outliner] Reparented node " + std::to_string(sourceId) + " under " + node.name);
        }
        ImGui::EndDragDropTarget();
    }

    // 1. Row Background Container (Selection > Hover > Zebra Striping)
    if (isSelected) {
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.24f)));
        dl->AddRectFilled(rMin, ImVec2(rMin.x + 3.0f, rMax.y), ImGui::ColorConvertFloat4ToU32(pal.accent));
    } else if (hovered) {
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(pal.bgElevated));
    } else if (rowIndex % 2 == 1) {
        dl->AddRectFilled(rMin, rMax, ImGui::ColorConvertFloat4ToU32(ImVec4(pal.bgHeader.x, pal.bgHeader.y, pal.bgHeader.z, 0.35f)));
    }

    if (clicked) {
        EditorState::Get().SetSelection(node.name, SceneGraph::GetTypeName(node.type));
        Logger::Get().Info("[Outliner] Selected node: " + node.name);
    }

    if (doubleClicked) {
        s_RenamingNodeId = node.id;
        snprintf(s_RenameBuf, sizeof(s_RenameBuf), "%s", node.name.c_str());
    }

    // 2. Persistent Vertical Guidelines connecting parent rows to children
    for (int d = 1; d <= depth; ++d) {
        float guideX = rMin.x + (d * indentPerLevel) - 8.0f;
        dl->AddLine(ImVec2(guideX, rMin.y), ImVec2(guideX, rMax.y), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    }

    float currentX = rMin.x + (depth * indentPerLevel) + 4.0f;

    // Visibility Toggle (Eye Dot)
    float visSize = 14.0f;
    ImVec2 visCenter(currentX + visSize * 0.5f, rMin.y + rowHeight * 0.5f);
    ImU32 eyeColor = node.visible ? ImGui::ColorConvertFloat4ToU32(pal.accent) : ImGui::ColorConvertFloat4ToU32(pal.textDisabled);
    dl->AddCircleFilled(visCenter, 3.5f, eyeColor);
    if (!node.visible) {
        dl->AddLine(ImVec2(visCenter.x - 4.0f, visCenter.y + 4.0f), ImVec2(visCenter.x + 4.0f, visCenter.y - 4.0f), ImGui::ColorConvertFloat4ToU32(pal.textDisabled), 1.5f);
    }

    ImRect visRect(ImVec2(currentX, rMin.y), ImVec2(currentX + visSize, rMax.y));
    if (ImGui::IsMouseClicked(0) && visRect.Contains(ImGui::GetMousePos())) {
        SceneGraph::Get().ToggleVisibility(node.id);
    }
    currentX += visSize + 4.0f;

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
            ImVec2 p1(currentX + 2.0f, centerY - 2.0f);
            ImVec2 p2(currentX + 10.0f, centerY - 2.0f);
            ImVec2 p3(currentX + 6.0f, centerY + 3.0f);
            dl->AddTriangleFilled(p1, p2, p3, caretColor);
        } else {
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

    ImVec4 iconColor = GetSceneNodeTypeColor(node.type);

    if (node.type == SceneNodeType::Folder) {
        ImU32 folderCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.95f, 0.75f, 0.35f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 3.0f), ImVec2(iconBoxMax.x - 1.0f, iconBoxMax.y - 1.0f), folderCol, 2.0f);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 1.0f, iconBoxMin.y + 1.0f), ImVec2(iconBoxMin.x + 7.0f, iconBoxMin.y + 4.0f), folderCol, 1.0f);
    } else if (node.type == SceneNodeType::Light) {
        ImU32 lightCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.96f, 0.82f, 0.28f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.5f, lightCol);
    } else if (node.type == SceneNodeType::Camera) {
        ImU32 camCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.40f, 0.85f, 0.50f, 1.0f));
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 4.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), camCol, 2.0f);
    } else if (node.type == SceneNodeType::Audio) {
        ImU32 audioCol = ImGui::ColorConvertFloat4ToU32(ImVec4(0.85f, 0.50f, 0.90f, 1.0f));
        dl->AddCircleFilled(ImVec2(iconBoxMin.x + 8.0f, iconBoxMin.y + 8.0f), 5.0f, audioCol);
    } else {
        ImU32 meshCol = ImGui::ColorConvertFloat4ToU32(iconColor);
        dl->AddRectFilled(ImVec2(iconBoxMin.x + 2.0f, iconBoxMin.y + 2.0f), ImVec2(iconBoxMax.x - 2.0f, iconBoxMax.y - 2.0f), meshCol, 2.0f);
    }

    currentX += iconBoxSize + 6.0f;

    // 5. Item Name / Inline Rename Input
    float maxNameX = rMax.x - typeColWidth - 10.0f;
    if (s_RenamingNodeId == node.id) {
        ImGui::SetCursorScreenPos(ImVec2(currentX, rMin.y + 1.0f));
        ImGui::SetNextItemWidth(maxNameX - currentX);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0f, 1.0f));
        if (ImGui::InputText("##InlineRename", s_RenameBuf, sizeof(s_RenameBuf), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            if (s_RenameBuf[0] != '\0') {
                std::string oldName = node.name;
                SceneGraph::Get().RenameNode(node.id, s_RenameBuf);
                if (EditorState::Get().selectedNodeName == oldName) {
                    EditorState::Get().selectedNodeName = s_RenameBuf;
                }
            }
            s_RenamingNodeId = 0;
        }
        if (!ImGui::IsItemActive() && ImGui::IsMouseClicked(0)) {
            s_RenamingNodeId = 0;
        }
        ImGui::PopStyleVar();
    } else {
        ImVec4 nameColor = isSelected ? pal.textPrimary : (node.visible ? pal.textPrimary : pal.textDisabled);
        ImVec2 namePos = ImVec2(currentX, rMin.y + (rowHeight - ImGui::GetTextLineHeight()) * 0.5f);

        dl->PushClipRect(rMin, ImVec2(maxNameX, rMax.y), true);
        dl->AddText(namePos, ImGui::ColorConvertFloat4ToU32(nameColor), node.name.c_str());
        dl->PopClipRect();
    }

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
    // ROW 1: SEARCH INPUT
    // ====================================================================
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(24.0f, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, pal.bgElevated);

    ImGui::SetNextItemWidth(-1.0f);
    ImGui::InputTextWithHint("##OutlinerSearch", "Search scene hierarchy...", s_OutlinerSearch, sizeof(s_OutlinerSearch), ImGuiInputTextFlags_EscapeClearsAll);

    ImVec2 sMin = ImGui::GetItemRectMin();
    ImVec2 sMax = ImGui::GetItemRectMax();

    ImDrawList* dl = ImGui::GetWindowDrawList();

    dl->AddCircleFilled(ImVec2(sMin.x + 12.0f, (sMin.y + sMax.y) * 0.5f), 3.5f, ImGui::ColorConvertFloat4ToU32(pal.textSecondary));

    float chevronX = sMax.x - 16.0f;
    float cY = (sMin.y + sMax.y) * 0.5f;
    ImU32 chevCol = ImGui::ColorConvertFloat4ToU32(pal.textSecondary);
    dl->AddTriangleFilled(
        ImVec2(chevronX, cY - 2.0f),
        ImVec2(chevronX + 8.0f, cY - 2.0f),
        ImVec2(chevronX + 4.0f, cY + 3.0f), chevCol);

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

    ImGui::PushStyleColor(ImGuiCol_Text, s_FilterVisibleOnly ? pal.accent : pal.textSecondary);
    if (ImGui::Button("\xE2\x97\x8F##VisToggle", ImVec2(20.0f, 20.0f))) {
        s_FilterVisibleOnly = !s_FilterVisibleOnly;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Visibility Filter");
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Text, s_FilterPinnedOnly ? pal.accent : pal.textSecondary);
    if (ImGui::Button("\xE2\x98\x85##PinToggle", ImVec2(20.0f, 20.0f))) {
        s_FilterPinnedOnly = !s_FilterPinnedOnly;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Pinned Items");
    ImGui::PopStyleColor();

    ImGui::PopStyleColor(2);

    ImGui::SameLine(0.0f, 6.0f);
    ImVec2 divP = ImGui::GetCursorScreenPos();
    float divY1 = divP.y + 3.0f;
    float divY2 = divY1 + 14.0f;
    dl->AddLine(ImVec2(divP.x, divY1), ImVec2(divP.x, divY2), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);
    ImGui::Dummy(ImVec2(1.0f, 0.0f));
    ImGui::SameLine(0.0f, 6.0f);

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
        // Drag-and-Drop Target for Root level unparenting
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_NODE")) {
                uint64_t sourceId = *(const uint64_t*)payload->Data;
                SceneGraph::Get().ReparentNode(sourceId, 0); // Unparent to root
                Logger::Get().Info("[Outliner] Unparented node " + std::to_string(sourceId) + " to root level");
            }
            ImGui::EndDragDropTarget();
        }

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

        // Hotkey shortcuts for Outliner: Del (Delete), Ctrl+D (Duplicate), F2 (Rename)
        if (ImGui::IsWindowFocused() && !EditorState::Get().selectedNodeName.empty()) {
            const SceneNode* selNode = SceneGraph::Get().FindNode(EditorState::Get().selectedNodeName);
            if (ImGui::IsKeyPressed(ImGuiKey_F2) && selNode) {
                s_RenamingNodeId = selNode->id;
                snprintf(s_RenameBuf, sizeof(s_RenameBuf), "%s", selNode->name.c_str());
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                SceneGraph::Get().RemoveNode(EditorState::Get().selectedNodeName);
                Logger::Get().Info("[Outliner] Deleted actor via Del key: " + EditorState::Get().selectedNodeName);
                EditorState::Get().ClearSelection();
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
                if (ImGui::BeginMenu("3D Object")) {
                    if (ImGui::MenuItem("Cube")) {
                        SceneNode cubeNode;
                        cubeNode.name = "Cube_" + std::to_string(rand() % 1000);
                        cubeNode.type = SceneNodeType::Actor;
                        cubeNode.location[0] = 0.0f; cubeNode.location[1] = 1.0f; cubeNode.location[2] = 0.0f;
                        cubeNode.meshPath = "Engine/DefaultCube";
                        cubeNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(cubeNode);
                        EditorState::Get().SetSelection(cubeNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Sphere")) {
                        SceneNode sphereNode;
                        sphereNode.name = "Sphere_" + std::to_string(rand() % 1000);
                        sphereNode.type = SceneNodeType::Actor;
                        sphereNode.location[0] = 0.0f; sphereNode.location[1] = 1.0f; sphereNode.location[2] = 0.0f;
                        sphereNode.meshPath = "Engine/DefaultSphere";
                        sphereNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(sphereNode);
                        EditorState::Get().SetSelection(sphereNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Cylinder")) {
                        SceneNode cylNode;
                        cylNode.name = "Cylinder_" + std::to_string(rand() % 1000);
                        cylNode.type = SceneNodeType::Actor;
                        cylNode.location[0] = 0.0f; cylNode.location[1] = 1.0f; cylNode.location[2] = 0.0f;
                        cylNode.meshPath = "Engine/DefaultCylinder";
                        cylNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(cylNode);
                        EditorState::Get().SetSelection(cylNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Plane")) {
                        SceneNode planeNode;
                        planeNode.name = "Plane_" + std::to_string(rand() % 1000);
                        planeNode.type = SceneNodeType::Actor;
                        planeNode.location[0] = 0.0f; planeNode.location[1] = 0.05f; planeNode.location[2] = 0.0f;
                        planeNode.meshPath = "Engine/DefaultPlane";
                        planeNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(planeNode);
                        EditorState::Get().SetSelection(planeNode.name, "Actor");
                    }
                    if (ImGui::MenuItem("Cone")) {
                        SceneNode coneNode;
                        coneNode.name = "Cone_" + std::to_string(rand() % 1000);
                        coneNode.type = SceneNodeType::Actor;
                        coneNode.location[0] = 0.0f; coneNode.location[1] = 1.0f; coneNode.location[2] = 0.0f;
                        coneNode.meshPath = "Engine/DefaultCone";
                        coneNode.materialPath = "DefaultPBRMaterial";
                        SceneGraph::Get().AddNode(coneNode);
                        EditorState::Get().SetSelection(coneNode.name, "Actor");
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Light")) {
                    if (ImGui::MenuItem("Directional Sun Light")) {
                        SceneNode sunNode;
                        sunNode.name = "SunLight_" + std::to_string(rand() % 1000);
                        sunNode.type = SceneNodeType::Light;
                        sunNode.location[0] = 0.0f; sunNode.location[1] = 10.0f; sunNode.location[2] = 0.0f;
                        sunNode.rotation[0] = 53.0f; sunNode.rotation[1] = -59.0f; sunNode.rotation[2] = 0.0f;
                        SceneGraph::Get().AddNode(sunNode);
                        LightComponent lc; lc.lightType = 0; lc.intensity = 100000.0f; lc.color[0] = 1.0f; lc.color[1] = 0.95f; lc.color[2] = 0.85f;
                        ComponentRegistry::Get().AddComponent<LightComponent>(sunNode.id, lc);
                        EditorState::Get().SetSelection(sunNode.name, "Light");
                        Logger::Get().Info("[Outliner] Added Directional Sun Light actor");
                    }
                    if (ImGui::MenuItem("Point Light")) {
                        SceneNode ptNode;
                        ptNode.name = "PointLight_" + std::to_string(rand() % 1000);
                        ptNode.type = SceneNodeType::Light;
                        ptNode.location[0] = 0.0f; ptNode.location[1] = 3.0f; ptNode.location[2] = 0.0f;
                        SceneGraph::Get().AddNode(ptNode);
                        LightComponent lc; lc.lightType = 1; lc.intensity = 2500.0f; lc.range = 15.0f; lc.color[0] = 1.0f; lc.color[1] = 0.90f; lc.color[2] = 0.70f;
                        ComponentRegistry::Get().AddComponent<LightComponent>(ptNode.id, lc);
                        EditorState::Get().SetSelection(ptNode.name, "Light");
                        Logger::Get().Info("[Outliner] Added Point Light actor");
                    }
                    if (ImGui::MenuItem("Spot Light")) {
                        SceneNode spotNode;
                        spotNode.name = "SpotLight_" + std::to_string(rand() % 1000);
                        spotNode.type = SceneNodeType::Light;
                        spotNode.location[0] = 0.0f; spotNode.location[1] = 3.0f; spotNode.location[2] = 0.0f;
                        spotNode.rotation[0] = -45.0f; spotNode.rotation[1] = 0.0f; spotNode.rotation[2] = 0.0f;
                        SceneGraph::Get().AddNode(spotNode);
                        LightComponent lc; lc.lightType = 2; lc.intensity = 5000.0f; lc.range = 25.0f; lc.color[0] = 1.0f; lc.color[1] = 0.90f; lc.color[2] = 0.70f;
                        ComponentRegistry::Get().AddComponent<LightComponent>(spotNode.id, lc);
                        EditorState::Get().SetSelection(spotNode.name, "Light");
                        Logger::Get().Info("[Outliner] Added Spot Light actor");
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("ZePhysics RigidBody Actor")) {
                    SceneNode physNode;
                    physNode.name = "RigidBody_" + std::to_string(rand() % 1000);
                    physNode.type = SceneNodeType::Actor;
                    physNode.location[0] = 0.0f; physNode.location[1] = 2.0f; physNode.location[2] = 0.0f;
                    SceneGraph::Get().AddNode(physNode);
                    RigidBodyComponent rb;
                    ComponentRegistry::Get().AddComponent<RigidBodyComponent>(physNode.id, rb);
                    EditorState::Get().SetSelection(physNode.name, "Actor");
                    Logger::Get().Info("[Outliner] Added ZePhysics RigidBody actor");
                }
                if (ImGui::MenuItem("Static Mesh Actor")) {
                    SceneNode meshNode;
                    meshNode.name = "StaticMesh_" + std::to_string(rand() % 1000);
                    meshNode.type = SceneNodeType::Actor;
                    meshNode.location[0] = 0.0f; meshNode.location[1] = 1.0f; meshNode.location[2] = 0.0f;
                    meshNode.meshPath = "primitives/cube.zmesh";
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
            if (ImGui::MenuItem("Rename Selected Actor", "F2", false, !EditorState::Get().selectedNodeName.empty())) {
                const SceneNode* target = SceneGraph::Get().FindNode(EditorState::Get().selectedNodeName);
                if (target) {
                    s_RenamingNodeId = target->id;
                    snprintf(s_RenameBuf, sizeof(s_RenameBuf), "%s", target->name.c_str());
                }
            }
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
    // ROW 4: PINNED FOOTER (Live Dynamic Actor Stats)
    // ====================================================================
    ImVec2 fMin = ImGui::GetCursorScreenPos();
    float fWidth = ImGui::GetContentRegionAvail().x;
    ImVec2 fMax = ImVec2(fMin.x + fWidth, fMin.y + footerHeight);

    dl->AddLine(fMin, ImVec2(fMax.x, fMin.y), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f);

    uint32_t totalActors = 0;
    uint32_t visibleActors = 0;
    CountNodesRecursive(SceneGraph::Get().GetRootNodes(), totalActors, visibleActors);
    uint32_t selectedCount = EditorState::Get().selectedNodeName.empty() ? 0 : 1;

    char footerBuf[128];
    snprintf(footerBuf, sizeof(footerBuf), "%u actors (%u visible, %u selected)", totalActors, visibleActors, selectedCount);

    float textCenterY = fMin.y + (footerHeight - ImGui::GetTextLineHeight()) * 0.5f;
    ImGui::SetCursorPosY(textCenterY - ImGui::GetWindowPos().y);
    ImGui::SetCursorPosX(8.0f);
    ImGui::TextColored(pal.textDisabled, "%s", footerBuf);

    ImGui::End();
}

} // namespace EngineEditor
