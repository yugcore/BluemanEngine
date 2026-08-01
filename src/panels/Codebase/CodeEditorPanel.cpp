#include "CodeEditorPanel.h"
#include "CodeHighlighter.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Fonts.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_set>
#include <algorithm>

namespace EngineEditor {

struct Document {
    std::string filename;
    std::string content;
    std::unordered_set<int> breakpoints;
    int cursorLine = 1;
    int cursorCol = 1;
    bool isOpen = true;
};

static std::vector<Document> s_Documents = {
    {
        "Main.cpp",
        "#include <iostream>\n"
        "#include \"Engine.h\"\n\n"
        "// ZeGFX Primary Application Entry Point\n"
        "int main(int argc, char** argv) {\n"
        "    std::cout << \"Initializing ZeGFX Engine v3.5...\" << std::endl;\n"
        "    Engine::Initialize();\n"
        "    Engine::Run();\n"
        "    Engine::Shutdown();\n"
        "    return 0;\n"
        "}\n",
        { 7 }, // Breakpoint on line 7
        6, 5, true
    },
    {
        "Renderer.cpp",
        "#include \"Renderer.h\"\n"
        "#include \"Engine.h\"\n\n"
        "// Hardware DXR Acceleration & Volumetrics\n"
        "void Renderer::RenderFrame() {\n"
        "    // Execute DXR Ray Tracing solve & Volumetric Lighting pipeline\n"
        "    Raytrace(Vec3(0, 5, 0), Vec3(0, -1, 0), 1000.0f);\n"
        "}\n",
        {},
        5, 1, true
    },
    {
        "Engine.h",
        "#pragma once\n\n"
        "namespace Engine {\n"
        "    void Initialize();\n"
        "    void Run();\n"
        "    void Shutdown();\n"
        "}\n",
        {},
        4, 1, true
    },
    {
        "game_logic.zl",
        "// ========================================================\n"
        "// Zelyn First-Class Script: Spatial Entity Controller\n"
        "// ========================================================\n"
        "@attribute(name: \"Player Speed\", category: \"Gameplay\")\n"
        "var move_speed: float = 12.5f\n\n"
        "func update(dt: float) -> void {\n"
        "    var pos: Vec3 = Vec3(0.0, 1.5, -4.0)\n"
        "    var hit: HitResult = Raytrace(pos, Vec3(0, -1, 0), 50.0f)\n"
        "    if (hit.hasHit) {\n"
        "        print(\"Raytraced surface hit at distance: \" + hit.distance)\n"
        "    }\n"
        "}\n\n"
        "func spawn_effects() -> Entity {\n"
        "    let e: Entity = spawn_entity(\"ParticleSystem\", Vec3(0, 0, 0))\n"
        "    return e\n"
        "}\n",
        { 8 }, // Breakpoint on line 8
        6, 1, true
    },
    {
        "player_controller.zyn",
        "// Zelyn Module: Player Controller Logic\n"
        "import Scene\n\n"
        "class PlayerController {\n"
        "    var health: int = 100\n"
        "    var is_active: bool = true\n\n"
        "    func on_start() -> void {\n"
        "        print(\"Player Controller Initialized.\")\n"
        "    }\n"
        "}\n",
        {},
        5, 1, true
    }
};

static int s_ActiveDocIndex = 0;

void OpenCodeDocument(const std::string& filename) {
    for (size_t i = 0; i < s_Documents.size(); ++i) {
        if (s_Documents[i].filename == filename) {
            s_Documents[i].isOpen = true;
            s_ActiveDocIndex = (int)i;
            EditorState::Get().activeCodeFileName = filename;
            return;
        }
    }
    // Create new document if not found
    LanguageType lang = CodeHighlighter::GetLanguageFromExtension(filename);
    std::string defaultContent = (lang == LanguageType::Zelyn) ?
        "// Zelyn Script\nfunc main() -> void {\n    print(\"Hello Zelyn!\")\n}\n" :
        "// C++ Source\n#include <iostream>\n\nvoid run() {\n}\n";
    s_Documents.push_back({ filename, defaultContent, {}, 1, 1, true });
    s_ActiveDocIndex = (int)s_Documents.size() - 1;
    EditorState::Get().activeCodeFileName = filename;
}

static std::vector<std::string> SplitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }
    if (lines.empty()) lines.push_back("");
    return lines;
}

void RenderCodeEditorPanel(bool* pOpen) {
    if (!ImGui::Begin("Code Editor", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    // Top-Right Ghosted Toggle Widget (Point 5)
    float availWidthHeader = ImGui::GetContentRegionAvail().x;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + availWidthHeader - 28.0f);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.textDisabled);
    if (ImGui::Button("☰##MinimapToggle", ImVec2(24.0f, 20.0f))) {
        // Toggle minimap pane
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle Code Minimap & Overview");
    ImGui::PopStyleColor(4);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 24.0f);

    // Render Tab Strip with Clean Colors & Accent Underline
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 6.0f));
    if (ImGui::BeginTabBar("CodeEditorTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
        for (int i = 0; i < (int)s_Documents.size(); ++i) {
            auto& doc = s_Documents[i];
            if (!doc.isOpen) continue;

            LanguageType lang = CodeHighlighter::GetLanguageFromExtension(doc.filename);

            // Clean tab label without emojis or badges
            std::string tabLabel = doc.filename + "##Tab" + std::to_string(i);
            
            ImGui::PushStyleColor(ImGuiCol_Text, pal.textPrimary);
            bool isSelected = ImGui::BeginTabItem(tabLabel.c_str(), &doc.isOpen);
            ImGui::PopStyleColor();

            if (isSelected) {
                s_ActiveDocIndex = i;
                EditorState::Get().activeCodeFileName = doc.filename;
                EditorState::Get().activeCodeLine = doc.cursorLine;
                EditorState::Get().activeCodeColumn = doc.cursorCol;

                // Active Tab Accent Underline
                ImVec2 tabMin = ImGui::GetItemRectMin();
                ImVec2 tabMax = ImGui::GetItemRectMax();
                ImU32 underlineCol = ImGui::ColorConvertFloat4ToU32(pal.accent);
                ImGui::GetWindowDrawList()->AddRectFilled(
                    ImVec2(tabMin.x, tabMax.y - 2.0f),
                    ImVec2(tabMax.x, tabMax.y),
                    underlineCol
                );

                // Document Content Viewport
                ImGui::Spacing();
                
                std::vector<std::string> lines = SplitLines(doc.content);
                float fontHeight = ImGui::GetTextLineHeightWithSpacing();
                float lineTextHeight = ImGui::GetTextLineHeight();

                // Setup Child Viewport for Editor with Scrollbars & Minimap
                float fullAvailWidth = ImGui::GetContentRegionAvail().x;
                float minimapWidth = 54.0f;
                float editorWidth = fullAvailWidth - minimapWidth - 8.0f;

                ImGui::PushStyleColor(ImGuiCol_ChildBg, pal.bgBase);
                if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

                if (ImGui::BeginChild("EditorMainPane", ImVec2(editorWidth, -1.0f), true, ImGuiWindowFlags_HorizontalScrollbar)) {
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 childPos = ImGui::GetCursorScreenPos();

                    for (size_t l = 0; l < lines.size(); ++l) {
                        int lineNum = (int)l + 1;
                        bool isCurrentLine = (lineNum == doc.cursorLine);

                        // Line Row Background Highlight for active line - centered in Y axis
                        ImVec2 linePos = ImGui::GetCursorScreenPos();
                        if (isCurrentLine) {
                            float yCenterOffset = (fontHeight - lineTextHeight) * 0.5f;
                            drawList->AddRectFilled(
                                ImVec2(linePos.x, linePos.y - yCenterOffset),
                                ImVec2(linePos.x + ImGui::GetContentRegionAvail().x, linePos.y + fontHeight - yCenterOffset),
                                ImGui::ColorConvertFloat4ToU32(ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.14f))
                            );
                        }

                        // 1. Breakpoint Gutter Column (20px)
                        ImGui::PushID((int)l * 100);
                        bool hasBp = (doc.breakpoints.count(lineNum) > 0);
                        ImVec2 bpPos = ImVec2(linePos.x + 6.0f, linePos.y + fontHeight * 0.5f);
                        if (hasBp) {
                            drawList->AddCircleFilled(bpPos, 5.0f, IM_COL32(235, 65, 65, 255));
                        } else {
                            // Subtle hover dot preview
                            if (ImGui::IsMouseHoveringRect(ImVec2(linePos.x, linePos.y), ImVec2(linePos.x + 20.0f, linePos.y + fontHeight))) {
                                drawList->AddCircleFilled(bpPos, 4.0f, IM_COL32(180, 80, 80, 140));
                                if (ImGui::IsMouseClicked(0)) {
                                    doc.breakpoints.insert(lineNum);
                                }
                            }
                        }
                        if (hasBp && ImGui::IsMouseHoveringRect(ImVec2(linePos.x, linePos.y), ImVec2(linePos.x + 20.0f, linePos.y + fontHeight))) {
                            if (ImGui::IsMouseClicked(0)) {
                                doc.breakpoints.erase(lineNum);
                            }
                        }
                        ImGui::Dummy(ImVec2(18.0f, fontHeight));
                        ImGui::SameLine(0.0f, 4.0f);
                        ImGui::PopID();

                        // 2. Line Number Gutter Column with generous spacing
                        char lineStr[16];
                        snprintf(lineStr, sizeof(lineStr), "%3d", lineNum);
                        ImVec4 lineNumColor = isCurrentLine ? pal.textPrimary : pal.textDisabled;
                        ImGui::TextColored(lineNumColor, "%s", lineStr);

                        // Separator line between gutter and code with clean gap
                        float separatorX = linePos.x + 78.0f;
                        drawList->AddLine(
                            ImVec2(separatorX, linePos.y - 1.0f),
                            ImVec2(separatorX, linePos.y + fontHeight - 1.0f),
                            ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 1.0f
                        );

                        // Set code cursor X with spacious gap after separator
                        ImGui::SetCursorScreenPos(ImVec2(separatorX + 16.0f, linePos.y));

                        // 3. Syntax Highlighted Line Rendering & Diagnostics
                        std::string diagError;
                        bool hasDiag = CodeHighlighter::GetDiagnosticError(doc.filename, lineNum, diagError);

                        ImVec2 codeStartPos = ImGui::GetCursorScreenPos();
                        CodeHighlighter::RenderHighlightedLine(lines[l], lang);

                        // Render Red Squiggle under error line
                        if (hasDiag) {
                            ImVec2 codeEndPos = ImVec2(codeStartPos.x + ImGui::CalcTextSize(lines[l].c_str()).x, codeStartPos.y + fontHeight);
                            if (codeEndPos.x <= codeStartPos.x) codeEndPos.x = codeStartPos.x + 80.0f;
                            
                            // Squiggle wave
                            float startX = codeStartPos.x;
                            float endX = codeEndPos.x;
                            float waveY = codeEndPos.y - 2.0f;
                            for (float x = startX; x < endX; x += 4.0f) {
                                float nextX = std::min(x + 4.0f, endX);
                                float yOffset = ((int)((x - startX) / 4.0f) % 2 == 0) ? -2.0f : 0.0f;
                                float nextYOffset = ((int)((nextX - startX) / 4.0f) % 2 == 0) ? -2.0f : 0.0f;
                                drawList->AddLine(
                                    ImVec2(x, waveY + yOffset),
                                    ImVec2(nextX, waveY + nextYOffset),
                                    IM_COL32(240, 70, 70, 255), 1.5f
                                );
                            }

                            if (ImGui::IsMouseHoveringRect(codeStartPos, codeEndPos)) {
                                ImGui::BeginTooltip();
                                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "[Problem Diagnostic]");
                                ImGui::TextUnformatted(diagError.c_str());
                                ImGui::EndTooltip();
                            }
                        }

                        // Allow clicking line to set cursor line
                        if (ImGui::IsMouseHoveringRect(linePos, ImVec2(linePos.x + editorWidth, linePos.y + fontHeight))) {
                            if (ImGui::IsMouseClicked(0)) {
                                doc.cursorLine = lineNum;
                                EditorState::Get().activeCodeLine = doc.cursorLine;
                            }
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::SameLine(0.0f, 8.0f);

                // 4. Code Minimap Pane
                if (ImGui::BeginChild("EditorMinimapPane", ImVec2(minimapWidth, -1.0f), true, ImGuiWindowFlags_NoScrollbar)) {
                    ImDrawList* mmDrawList = ImGui::GetWindowDrawList();
                    ImVec2 mmPos = ImGui::GetCursorScreenPos();
                    
                    // Minimap background track
                    mmDrawList->AddRectFilled(mmPos, ImVec2(mmPos.x + minimapWidth, mmPos.y + ImGui::GetContentRegionAvail().y), IM_COL32(20, 22, 28, 255));

                    // Micro line representations
                    float mmY = mmPos.y + 4.0f;
                    for (size_t l = 0; l < lines.size(); ++l) {
                        float len = std::min((float)lines[l].length() * 1.5f, minimapWidth - 8.0f);
                        if (len > 0.0f) {
                            ImU32 mmColor = IM_COL32(180, 180, 180, 180);
                            if (doc.breakpoints.count((int)l + 1)) mmColor = IM_COL32(235, 65, 65, 255);

                            mmDrawList->AddRectFilled(
                                ImVec2(mmPos.x + 4.0f, mmY),
                                ImVec2(mmPos.x + 4.0f + len, mmY + 2.0f),
                                mmColor
                            );
                        }
                        mmY += 3.5f;
                    }

                    // Viewport Highlight Rectangle in Minimap
                    mmDrawList->AddRect(
                        ImVec2(mmPos.x + 2.0f, mmPos.y + doc.cursorLine * 3.5f - 4.0f),
                        ImVec2(mmPos.x + minimapWidth - 2.0f, mmPos.y + doc.cursorLine * 3.5f + 16.0f),
                        ImGui::ColorConvertFloat4ToU32(pal.accent), 0.0f, 0, 1.5f
                    );
                }
                ImGui::EndChild();

                if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();
                ImGui::PopStyleColor();

                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::PopStyleVar(); // FramePadding for tabs

    ImGui::End();
}

} // namespace EngineEditor
