#include "CodeBottomDockPanel.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Fonts.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>
#include <vector>

namespace EngineEditor {

static char s_TerminalInputBuffer[256] = "";
static std::vector<std::string> s_TerminalHistory = {
    "[ZeGFX Interactive Shell v3.5 - Zelyn Kernel Active]",
    "Type 'help' or 'zelyn --version' for available commands.",
    "zelyn> zelyn --version",
    "Zelyn Compiler v1.0.4 (x86_64-pc-windows-msvc) [Built with DX12 ZeGFX]",
    "zelyn> check game_logic.zl",
    "[Zelyn Diagnostic] game_logic.zl: 1 Warning (Line 15), 0 Severe Errors."
};

void RenderCodeBottomDockPanel(bool* pOpen) {
    if (!ImGui::Begin("Terminal", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

    if (ImGui::BeginTabBar("CodeBottomDockTabBar")) {
        // Tab 1: Terminal with prompt and blinking cursor
        if (ImGui::BeginTabItem("Terminal")) {
            ImGui::Spacing();

            // Terminal Output Window
            ImGui::BeginChild("TerminalOutputPane", ImVec2(0, -32.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
            for (const auto& line : s_TerminalHistory) {
                if (line.rfind("zelyn>", 0) == 0) {
                    // Prompt in system accent purple
                    ImGui::TextColored(pal.accent, "%s", line.c_str());
                } else if (line.find("Error") != std::string::npos) {
                    ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "%s", line.c_str());
                } else if (line.find("Warning") != std::string::npos) {
                    ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.25f, 1.0f), "%s", line.c_str());
                } else if (line.find("successfully") != std::string::npos || line.find("Built with") != std::string::npos) {
                    ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "%s", line.c_str());
                } else {
                    ImGui::TextColored(pal.textPrimary, "%s", line.c_str());
                }
            }
            // Auto scroll to bottom
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.0f) {
                ImGui::SetScrollHereY(1.0f);
            }
            ImGui::EndChild();

            ImGui::Separator();

            // Prompt Line & Blinking Cursor
            ImGui::TextColored(pal.accent, "zelyn>");
            ImGui::SameLine(0.0f, 6.0f);

            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 30.0f);
            
            bool reclaimFocus = false;
            if (ImGui::InputText("##TermInput", s_TerminalInputBuffer, sizeof(s_TerminalInputBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
                if (s_TerminalInputBuffer[0] != '\0') {
                    std::string cmd = s_TerminalInputBuffer;
                    s_TerminalHistory.push_back("zelyn> " + cmd);
                    if (cmd == "help") {
                        s_TerminalHistory.push_back("Available commands: zelyn build, zelyn run, check, clear, status");
                    } else if (cmd == "clear") {
                        s_TerminalHistory.clear();
                    } else {
                        s_TerminalHistory.push_back("Executed command: '" + cmd + "' successfully.");
                    }
                    s_TerminalInputBuffer[0] = '\0';
                }
                reclaimFocus = true;
            }
            if (reclaimFocus) ImGui::SetKeyboardFocusHere(-1);

            // Blinking cursor representation
            static float cursorTimer = 0.0f;
            cursorTimer += ImGui::GetIO().DeltaTime;
            if (((int)(cursorTimer * 2.0f) % 2) == 0) {
                ImGui::SameLine(0.0f, 0.0f);
                ImGui::TextColored(pal.accent, "_");
            }

            ImGui::PopStyleColor(2);
            ImGui::EndTabItem();
        }

        // Tab 2: Build Output (Strict Minimal Colors)
        if (ImGui::BeginTabItem("Build Output")) {
            ImGui::Spacing();
            ImGui::BeginChild("BuildOutputPane");
            ImGui::TextColored(pal.textPrimary, "[ZeGFX Build System v3.5]");
            ImGui::TextColored(pal.textSecondary, "Compiling C++ engine targets and Zelyn scripts...");
            ImGui::TextColored(pal.textPrimary, "  -> Compiling RayTracingPipeline.hlsl (DXIL)... Done.");
            ImGui::TextColored(pal.textPrimary, "  -> Compiling game_logic.zl AST... Done (0.04s).");
            ImGui::TextColored(pal.textPrimary, "  -> Compiling player_controller.zyn module... Done (0.02s).");
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "BUILD SUCCESSFUL - 0 Errors, 1 Warning (0.62s total)");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // Tab 3: Git (Strict Minimal Colors)
        if (ImGui::BeginTabItem("Git")) {
            ImGui::Spacing();
            ImGui::BeginChild("GitPane");
            ImGui::TextColored(pal.textPrimary, "On branch feature/volumetrics");
            ImGui::TextColored(pal.textSecondary, "Your branch is up to date with 'origin/feature/volumetrics'.");
            ImGui::Spacing();
            ImGui::TextColored(pal.textPrimary, "Changes ready for commit:");
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "  modified:   src/panels/Codebase/CodeEditorPanel.cpp");
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "  modified:   src/panels/Codebase/ProjectExplorerPanel.cpp");
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "  modified:   scripts/game_logic.zl");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // Tab 4: Problems (Isolated Alert Red/Orange Color)
        std::string probTabName = "Problems (2)";
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.35f, 0.35f, 1.0f));
        bool isProbOpen = ImGui::BeginTabItem(probTabName.c_str());
        ImGui::PopStyleColor();

        if (isProbOpen) {
            ImGui::Spacing();
            ImGui::BeginChild("ProblemsPane");
            ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "● 1 Error  |  ▲ 1 Warning");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "● [Error] game_logic.zl:6");
            ImGui::SameLine(0.0f, 12.0f);
            ImGui::TextColored(pal.textPrimary, "Syntax Error: Missing semicolon or closing parenthesis after Raytrace call.");

            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.95f, 0.70f, 0.25f, 1.0f), "▲ [Warning] Main.cpp:15");
            ImGui::SameLine(0.0f, 12.0f);
            ImGui::TextColored(pal.textPrimary, "Warning: Engine::Initialize should be called before configuring viewports.");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        // Tab 5: Debug Console (Strict Minimal Colors)
        if (ImGui::BeginTabItem("Debug Console")) {
            ImGui::Spacing();
            ImGui::BeginChild("DebugPane");
            ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.45f, 1.0f), "[DX12 Debug Layer] Device created successfully.");
            ImGui::TextColored(pal.textPrimary, "[Zelyn VM Debugger] Connected to local session at 127.0.0.1:9002.");
            ImGui::TextColored(pal.textPrimary, "[Zelyn VM] Breakpoint hit at game_logic.zl:8 in func update(dt: float)");
            ImGui::TextColored(pal.textSecondary, "  -> move_speed = 12.5f");
            ImGui::TextColored(pal.textSecondary, "  -> pos = (0.0, 1.5, -4.0)");
            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();

    ImGui::End();
}

} // namespace EngineEditor
