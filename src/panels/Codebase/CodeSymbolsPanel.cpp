#include "CodeSymbolsPanel.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

void RenderCodeSymbolsPanel(bool* pOpen) {
    if (!ImGui::Begin("Symbols", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();
    const std::string& activeFile = EditorState::Get().activeCodeFileName;

    if (ImGui::BeginTabBar("CodeSymbolsTabBar")) {
        // Tab 1: Symbols
        if (ImGui::BeginTabItem("Symbols")) {
            ImGui::Spacing();
            if (activeFile.empty()) {
                ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);
                ImGui::TextColored(pal.textDisabled, "No file open.");
            } else {
                ImGui::TextColored(pal.accent, "Active File: %s", activeFile.c_str());
                ImGui::Separator();
                ImGui::Spacing();

                if (activeFile == "Main.cpp") {
                    if (ImGui::TreeNodeEx("#includes", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(pal.textSecondary, "[H] <iostream>");
                        ImGui::TextColored(pal.textSecondary, "[H] \"Engine.h\"");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNodeEx("Functions", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(0.86f, 0.82f, 0.54f, 1.0f), "[F] main(int argc, char** argv) : int (Ln 5)");
                        ImGui::TreePop();
                    }
                } else if (activeFile == "Renderer.cpp") {
                    if (ImGui::TreeNodeEx("#includes", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(pal.textSecondary, "[H] \"Renderer.h\"");
                        ImGui::TextColored(pal.textSecondary, "[H] \"Engine.h\"");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNodeEx("Methods", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(0.86f, 0.82f, 0.54f, 1.0f), "[M] Renderer::RenderFrame() : void (Ln 5)");
                        ImGui::TreePop();
                    }
                } else if (activeFile == "Engine.h") {
                    if (ImGui::TreeNodeEx("namespace Engine", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(0.86f, 0.82f, 0.54f, 1.0f), "[F] Initialize() : void (Ln 4)");
                        ImGui::TextColored(ImVec4(0.86f, 0.82f, 0.54f, 1.0f), "[F] Run() : void (Ln 5)");
                        ImGui::TextColored(ImVec4(0.86f, 0.82f, 0.54f, 1.0f), "[F] Shutdown() : void (Ln 6)");
                        ImGui::TreePop();
                    }
                } else if (activeFile == "game_logic.zl") {
                    if (ImGui::TreeNodeEx("Attributes & Variables", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(0.54f, 0.87f, 1.0f, 1.0f), "[V] move_speed : float = 12.5f (Ln 5)");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNodeEx("Zelyn Functions", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(1.00f, 0.80f, 0.42f, 1.0f), "[fn] update(dt: float) -> void (Ln 7)");
                        ImGui::TextColored(ImVec4(1.00f, 0.80f, 0.42f, 1.0f), "[fn] spawn_effects() -> Entity (Ln 15)");
                        ImGui::TreePop();
                    }
                } else if (activeFile == "player_controller.zyn") {
                    if (ImGui::TreeNodeEx("class PlayerController", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::TextColored(ImVec4(0.54f, 0.87f, 1.0f, 1.0f), "[V] health : int = 100 (Ln 5)");
                        ImGui::TextColored(ImVec4(0.54f, 0.87f, 1.0f, 1.0f), "[V] is_active : bool = true (Ln 6)");
                        ImGui::TextColored(ImVec4(1.00f, 0.80f, 0.42f, 1.0f), "[fn] on_start() -> void (Ln 8)");
                        ImGui::TreePop();
                    }
                } else {
                    ImGui::TextColored(pal.textSecondary, "Symbols parsed (0 classes, 1 function)");
                }
            }
            ImGui::EndTabItem();
        }

        // Tab 2: References
        if (ImGui::BeginTabItem("References")) {
            ImGui::Spacing();
            if (activeFile.empty()) {
                ImGui::TextColored(pal.textDisabled, "No file open.");
            } else {
                ImGui::TextColored(pal.textSecondary, "References in %s:", activeFile.c_str());
                ImGui::BulletText("Engine::Initialize() -> Main.cpp (Ln 7)");
                ImGui::BulletText("Raytrace() -> game_logic.zl (Ln 9)");
            }
            ImGui::EndTabItem();
        }

        // Tab 3: Outline
        if (ImGui::BeginTabItem("Outline")) {
            ImGui::Spacing();
            if (activeFile.empty()) {
                ImGui::TextColored(pal.textDisabled, "No file open.");
            } else {
                ImGui::TextColored(pal.accent, "Document Structure Outline:");
                if (activeFile.find(".zl") != std::string::npos || activeFile.find(".zyn") != std::string::npos) {
                    ImGui::TextColored(ImVec4(0.78f, 0.57f, 0.92f, 1.0f), "  @attribute move_speed");
                    ImGui::TextColored(ImVec4(1.00f, 0.80f, 0.42f, 1.0f), "  func update()");
                    ImGui::TextColored(ImVec4(1.00f, 0.80f, 0.42f, 1.0f), "  func spawn_effects()");
                } else {
                    ImGui::TextColored(pal.textPrimary, "  #include headers");
                    ImGui::TextColored(pal.textPrimary, "  main() entrypoint");
                }
            }
            ImGui::EndTabItem();
        }

        // Tab 4: Call Hierarchy
        if (ImGui::BeginTabItem("Call Hierarchy")) {
            ImGui::Spacing();
            if (activeFile.empty()) {
                ImGui::TextColored(pal.textDisabled, "No file open.");
            } else {
                if (ImGui::TreeNodeEx("main()", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("  -> Engine::Initialize()");
                    ImGui::Text("  -> Engine::Run()");
                    ImGui::Text("  -> Engine::Shutdown()");
                    ImGui::TreePop();
                }
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
