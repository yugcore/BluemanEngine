#include "CodeEditorPanel.h"
#include "theme/Colors.h"
#include "theme/Fonts.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

static char s_MainCppBuffer[1024] = 
    "#include <iostream>\n"
    "#include \"Engine.h\"\n\n"
    "int main(int argc, char** argv) {\n"
    "    std::cout << \"Initializing ZeGFX Engine v3.5...\" << std::endl;\n"
    "    Engine::Initialize();\n"
    "    Engine::Run();\n"
    "    Engine::Shutdown();\n"
    "    return 0;\n"
    "}\n";

static char s_RendererBuffer[1024] = 
    "#include \"Renderer.h\"\n\n"
    "void Renderer::RenderFrame() {\n"
    "    // Execute DXR Ray Tracing solve & Volumetric Lighting pipeline\n"
    "}\n";

static char s_EngineHBuffer[1024] = 
    "#pragma once\n\n"
    "namespace Engine {\n"
    "    void Initialize();\n"
    "    void Run();\n"
    "    void Shutdown();\n"
    "}\n";

void RenderCodeEditorPanel(bool* pOpen) {
    if (!ImGui::Begin("Code Editor", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (ImGui::BeginTabBar("CodeEditorTabBar")) {
        if (ImGui::BeginTabItem("Main.cpp")) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgBase);
            if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);
            
            ImGui::InputTextMultiline("##MainCppEdit", s_MainCppBuffer, sizeof(s_MainCppBuffer),
                                      ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput);
            
            if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Renderer.cpp")) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgBase);
            if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

            ImGui::InputTextMultiline("##RendererEdit", s_RendererBuffer, sizeof(s_RendererBuffer),
                                      ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput);

            if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Engine.h")) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgBase);
            if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

            ImGui::InputTextMultiline("##EngineHEdit", s_EngineHBuffer, sizeof(s_EngineHBuffer),
                                      ImVec2(-1.0f, -1.0f), ImGuiInputTextFlags_AllowTabInput);

            if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();
            ImGui::PopStyleColor();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
