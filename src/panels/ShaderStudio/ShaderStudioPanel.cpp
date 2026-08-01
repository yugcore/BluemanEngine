#include "ShaderStudioPanel.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Fonts.h"
#include "theme/Metrics.h"

#include "shader_compiler.h"
#include "shader_source.h"

#include <imgui.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

namespace EngineEditor {

static float s_AlbedoColor[4] = { 0.85f, 0.85f, 0.88f, 1.0f };
static float s_Roughness = 0.35f;
static float s_Metallic = 0.80f;
static float s_NormalScale = 1.0f;
static float s_EmissiveIntensity = 0.0f;
static float s_UvTiling[2] = { 1.0f, 1.0f };

static const char* s_DefaultHlslCode =
"// ZeGFX DX12 PBR Surface Material Shader\n"
"struct PSInput {\n"
"    float4 position : SV_POSITION;\n"
"    float3 normal   : NORMAL;\n"
"    float2 uv       : TEXCOORD0;\n"
"    float3 viewDir  : TEXCOORD1;\n"
"};\n\n"
"cbuffer MaterialConstants : register(b0) {\n"
"    float4 albedoColor;\n"
"    float  roughness;\n"
"    float  metallic;\n"
"    float  normalScale;\n"
"    float  emissiveIntensity;\n"
"};\n\n"
"float4 PSMain(PSInput input) : SV_TARGET {\n"
"    float3 N = normalize(input.normal);\n"
"    float3 V = normalize(input.viewDir);\n"
"    float3 albedo = albedoColor.rgb;\n"
"    float NdotL = max(dot(N, float3(0.577, 0.577, 0.577)), 0.1f);\n"
"    float3 color = albedo * NdotL + float3(emissiveIntensity, emissiveIntensity, emissiveIntensity);\n"
"    return float4(color, 1.0f);\n"
"}\n";

static void CompileShaderInStudio(ShaderStudioData& shaderData) {
    if (shaderData.shaderSource.empty()) {
        shaderData.shaderSource = s_DefaultHlslCode;
    }

    shaderData.diagnostics.clear();
    shaderData.compileErrorStr.clear();

    // 1. Ensure temp directory exists and write current HLSL source code to disk
    try {
        std::filesystem::create_directories("Cache/shaders");
    } catch (...) {}

    std::string tempPath = "Cache/shaders/shader_studio_temp.hlsl";
    {
        std::ofstream outFile(tempPath, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write(shaderData.shaderSource.c_str(), shaderData.shaderSource.size());
            outFile.close();
        } else {
            shaderData.isCompiled = true;
            shaderData.lastCompileSucceeded = false;
            shaderData.compileStatusMsg = "Failed to write temp HLSL file to Cache/shaders/";
            ShaderStudioDiagnostic diag;
            diag.severity = "ERROR";
            diag.line = 0;
            diag.column = 0;
            diag.message = shaderData.compileStatusMsg;
            shaderData.diagnostics.push_back(diag);
            Logger::Get().Error("[ShaderStudio] " + shaderData.compileStatusMsg);
            return;
        }
    }

    // 2. Setup VFS and ShaderCompiler
    zegfx::VirtualFileSystem vfs;
    vfs.registerMount("", "");

    zegfx::ShaderCompiler compiler;
    std::string initErr;
    if (!compiler.initialize(initErr)) {
        shaderData.isCompiled = true;
        shaderData.lastCompileSucceeded = false;
        shaderData.compileStatusMsg = "DXC Compiler Init Failed: " + initErr;
        ShaderStudioDiagnostic diag;
        diag.severity = "ERROR";
        diag.line = 0;
        diag.column = 0;
        diag.message = shaderData.compileStatusMsg;
        shaderData.diagnostics.push_back(diag);
        Logger::Get().Error("[ShaderStudio] " + shaderData.compileStatusMsg);
        return;
    }

    // 3. Build ShaderCompileRequest
    zegfx::ShaderCompileRequest req;
    req.source.path = { tempPath };
    req.source.entryPoint = shaderData.entryPoint.empty() ? "PSMain" : shaderData.entryPoint;
    
    if (shaderData.targetProfile.find("vs") != std::string::npos) {
        req.source.stage = zegfx::ShaderStage::Vertex;
    } else if (shaderData.targetProfile.find("cs") != std::string::npos) {
        req.source.stage = zegfx::ShaderStage::Compute;
    } else {
        req.source.stage = zegfx::ShaderStage::Pixel;
    }

    req.targetProfile = shaderData.targetProfile;
    req.options.hlslVersion = 2021;
    req.options.optimize = true;
    req.options.warningsAsErrors = false;

    // 4. Compile with DXC
    zegfx::ShaderCompileResult result;
    auto tStart = std::chrono::high_resolution_clock::now();
    bool compiled = compiler.compile(req, vfs, result);
    auto tEnd = std::chrono::high_resolution_clock::now();

    shaderData.compileTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count();
    shaderData.compilerVersion = compiler.getCompilerVersion();
    shaderData.isCompiled = true;

    // 5. Parse Diagnostics
    for (const auto& d : result.diagnostics) {
        ShaderStudioDiagnostic diag;
        diag.severity = (d.severity == zegfx::ShaderDiagnosticSeverity::Error) ? "ERROR" :
                        (d.severity == zegfx::ShaderDiagnosticSeverity::Warning) ? "WARNING" : "NOTE";
        diag.line = (int)d.line;
        diag.column = (int)d.column;
        diag.message = d.message;
        shaderData.diagnostics.push_back(diag);
    }

    if (compiled && result.succeeded) {
        shaderData.lastCompileSucceeded = true;
        shaderData.dxilBytecodeSize = result.dxil.size();
        std::ostringstream ss;
        ss << "Compiled successfully to DXIL (" << result.dxil.size() << " bytes, " << shaderData.compileTimeMs << " ms)";
        shaderData.compileStatusMsg = ss.str();
        Logger::Get().Info("[ShaderStudio] DXC Live Shader Pass recompiled successfully. " + shaderData.compileStatusMsg);
    } else {
        shaderData.lastCompileSucceeded = false;
        shaderData.dxilBytecodeSize = 0;
        int errCount = 0;
        for (const auto& diag : shaderData.diagnostics) {
            if (diag.severity == "ERROR") errCount++;
        }
        std::ostringstream ss;
        ss << "DXC compilation failed with " << (errCount > 0 ? errCount : (int)shaderData.diagnostics.size()) << " diagnostic issue(s)";
        shaderData.compileStatusMsg = ss.str();
        if (!shaderData.diagnostics.empty()) {
            shaderData.compileErrorStr = shaderData.diagnostics[0].message;
        } else {
            shaderData.compileErrorStr = "Unknown HLSL compilation error";
        }
        Logger::Get().Error("[ShaderStudio] DXC compilation failed. " + shaderData.compileStatusMsg);
    }
}

void RenderShaderStudioPanel(bool* pOpen) {
    if (!ImGui::Begin("Shader Studio", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    auto& shaderData = EditorState::Get().shaderStudioData;

    // Initialize default HLSL code if empty
    if (shaderData.shaderSource.empty()) {
        shaderData.shaderSource = s_DefaultHlslCode;
    }

    // Active Material File & Settings Header
    ImGui::TextColored(pal.textSecondary, "ACTIVE HLSL SHADER MATERIAL");
    ImGui::TextColored(pal.textPrimary, "%s", shaderData.shaderName.empty() ? "CustomSurfaceShader.hlsl" : shaderData.shaderName.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Configuration Row: Entry Point & Target Profile
    ImGui::TextColored(pal.textSecondary, "DXC Configuration:");
    ImGui::SameLine();
    
    // Entry Point Input
    char entryBuf[64];
    strncpy(entryBuf, shaderData.entryPoint.c_str(), sizeof(entryBuf) - 1);
    entryBuf[sizeof(entryBuf) - 1] = '\0';
    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::InputText("Entry Point", entryBuf, sizeof(entryBuf))) {
        shaderData.entryPoint = entryBuf;
    }

    ImGui::SameLine(0.0f, 16.0f);

    // Target Profile Combo
    const char* profiles[] = { "ps_6_0", "vs_6_0", "cs_6_0" };
    int currentProfileIdx = shaderData.targetProfileIdx;
    ImGui::SetNextItemWidth(100.0f);
    if (ImGui::Combo("Target Profile", &currentProfileIdx, profiles, IM_ARRAYSIZE(profiles))) {
        shaderData.targetProfileIdx = currentProfileIdx;
        shaderData.targetProfile = profiles[currentProfileIdx];
    }

    ImGui::Spacing();

    // Section 1: HLSL Shader Source Code Viewport
    if (ImGui::CollapsingHeader("HLSL Shader Source Code", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);
        
        if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

        // Edit buffer for ImGui multiline text input
        static char codeBuf[32768];
        if (shaderData.shaderSource.size() < sizeof(codeBuf)) {
            strncpy(codeBuf, shaderData.shaderSource.c_str(), sizeof(codeBuf) - 1);
            codeBuf[sizeof(codeBuf) - 1] = '\0';
        }

        ImVec2 codeEditorSize(0.0f, 220.0f);
        if (ImGui::InputTextMultiline("##HlslSourceEditor", codeBuf, sizeof(codeBuf), codeEditorSize, ImGuiInputTextFlags_AllowTabInput)) {
            shaderData.shaderSource = codeBuf;
            if (shaderData.autoCompileOnEdit) {
                CompileShaderInStudio(shaderData);
            }
        }

        // Ctrl + Enter shortcut inside editor to trigger live compilation
        if (ImGui::IsItemFocused() && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            CompileShaderInStudio(shaderData);
        }

        if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Section 2: Action Buttons Row
    ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    if (ImGui::Button("Recompile Shader Pass (DXC)", ImVec2(210.0f, 28.0f))) {
        CompileShaderInStudio(shaderData);
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0.0f, 8.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    if (ImGui::Button("Save Material Asset", ImVec2(150.0f, 28.0f))) {
        Logger::Get().Info("[ShaderStudio] Saved material asset " + (shaderData.shaderName.empty() ? "CustomSurfaceShader.hlsl" : shaderData.shaderName) + " to disk.");
    }
    ImGui::PopStyleColor();

    ImGui::SameLine(0.0f, 8.0f);

    if (ImGui::Button("Reset Template", ImVec2(120.0f, 28.0f))) {
        shaderData.shaderSource = s_DefaultHlslCode;
        shaderData.entryPoint = "PSMain";
        shaderData.targetProfile = "ps_6_0";
        shaderData.targetProfileIdx = 0;
        CompileShaderInStudio(shaderData);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Section 3: Live DXC Compiler Diagnostics & Error Reporting
    if (ImGui::CollapsingHeader("DXC Live Compiler Diagnostics & Status", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        // Compiler Status Banner
        if (!shaderData.isCompiled) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 0.6f));
            ImGui::BeginChild("StatusBanner", ImVec2(0, 32.0f), true);
            ImGui::TextColored(pal.textSecondary, "[DXC READY] Click 'Recompile Shader Pass' or press Ctrl+Enter in code editor.");
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else if (shaderData.lastCompileSucceeded) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.4f, 0.15f, 0.7f));
            ImGui::BeginChild("StatusBanner", ImVec2(0, 32.0f), true);
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.5f, 1.0f), "[SUCCESS] %s", shaderData.compileStatusMsg.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.5f, 0.12f, 0.12f, 0.8f));
            ImGui::BeginChild("StatusBanner", ImVec2(0, 32.0f), true);
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "[ERROR] %s", shaderData.compileStatusMsg.c_str());
            ImGui::EndChild();
            ImGui::PopStyleColor();
        }

        ImGui::Spacing();

        // Diagnostics Log Table
        if (!shaderData.diagnostics.empty()) {
            ImGui::TextColored(pal.textSecondary, "Diagnostic Messages (%zu):", shaderData.diagnostics.size());
            
            static ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
            if (ImGui::BeginTable("DiagnosticsTable", 4, tableFlags, ImVec2(0, 140.0f))) {
                ImGui::TableSetupColumn("Severity", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("Line", ImGuiTableColumnFlags_WidthFixed, 50.0f);
                ImGui::TableSetupColumn("Col", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                for (const auto& diag : shaderData.diagnostics) {
                    ImGui::TableNextRow();

                    // Severity Column
                    ImGui::TableSetColumnIndex(0);
                    if (diag.severity == "ERROR") {
                        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "[ERROR]");
                    } else if (diag.severity == "WARNING") {
                        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.3f, 1.0f), "[WARN]");
                    } else {
                        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "[NOTE]");
                    }

                    // Line Column
                    ImGui::TableSetColumnIndex(1);
                    if (diag.line > 0) ImGui::Text("%d", diag.line);
                    else ImGui::Text("-");

                    // Column Column
                    ImGui::TableSetColumnIndex(2);
                    if (diag.column > 0) ImGui::Text("%d", diag.column);
                    else ImGui::Text("-");

                    // Message Column
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(diag.message.c_str());
                }

                ImGui::EndTable();
            }
        } else if (shaderData.isCompiled && shaderData.lastCompileSucceeded) {
            ImGui::TextColored(pal.textSecondary, "No diagnostic errors or warnings reported by DXC.");
        }

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Section 4: PBR Uniform Material Parameters
    if (ImGui::CollapsingHeader("PBR Material Uniform Parameters")) {
        ImGui::Indent(8.0f);

        ImGui::ColorEdit4("Albedo Tint", s_AlbedoColor);
        ImGui::SliderFloat("Roughness", &s_Roughness, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Metallic", &s_Metallic, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Normal Map Scale", &s_NormalScale, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Emissive Intensity", &s_EmissiveIntensity, 0.0f, 10.0f, "%.1f");
        ImGui::InputFloat2("UV Tiling (U, V)", s_UvTiling, "%.1f");

        ImGui::Unindent(8.0f);
    }

    // Section 5: Compiled DXIL Metadata
    if (shaderData.isCompiled && ImGui::CollapsingHeader("Compiled DXIL Metadata")) {
        ImGui::Indent(8.0f);
        ImGui::Text("Compiler Engine: %s", shaderData.compilerVersion.c_str());
        ImGui::Text("Target Profile:  %s", shaderData.targetProfile.c_str());
        ImGui::Text("Entry Point:     %s", shaderData.entryPoint.c_str());
        ImGui::Text("DXIL Bytecode:   %zu bytes", shaderData.dxilBytecodeSize);
        ImGui::Text("Compile Time:    %llu ms", shaderData.compileTimeMs);
        ImGui::Unindent(8.0f);
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace EngineEditor
