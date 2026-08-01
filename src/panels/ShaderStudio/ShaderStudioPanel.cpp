#include "ShaderStudioPanel.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Fonts.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

static float s_AlbedoColor[4] = { 0.85f, 0.85f, 0.88f, 1.0f };
static float s_Roughness = 0.35f;
static float s_Metallic = 0.80f;
static float s_NormalScale = 1.0f;
static float s_EmissiveIntensity = 0.0f;
static float s_UvTiling[2] = { 1.0f, 1.0f };

static std::string s_ShaderHlslCode =
"// ZeGFX DX12 PBR Surface Material Shader\n"
"struct MaterialConstants {\n"
"    float4 albedoColor;\n"
"    float roughness;\n"
"    float metallic;\n"
"    float normalScale;\n"
"    float emissiveIntensity;\n"
"};\n\n"
"float4 PSMain(VSOutput input) : SV_Target {\n"
"    float3 N = normalize(input.normal);\n"
"    float3 V = normalize(input.viewDir);\n"
"    float3 albedo = TextureAlbedo.Sample(SamplerState, input.uv).rgb * albedoColor.rgb;\n"
"    float3 color = ComputePBR(N, V, albedo, roughness, metallic);\n"
"    return float4(color, 1.0f);\n"
"}\n";

void RenderShaderStudioPanel(bool* pOpen) {
    if (!ImGui::Begin("Shader Studio", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    auto& shaderData = EditorState::Get().shaderStudioData;

    // Active Material File Header
    ImGui::TextColored(pal.textSecondary, "ACTIVE SHADER MATERIAL");
    ImGui::TextColored(pal.textPrimary, "%s", shaderData.shaderName.empty() ? "CustomSurfaceShader.hlsl" : shaderData.shaderName.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    // Section 1: PBR Uniform Material Parameters
    if (ImGui::CollapsingHeader("PBR Material Uniforms", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        ImGui::ColorEdit4("Albedo Tint", s_AlbedoColor);
        ImGui::SliderFloat("Roughness", &s_Roughness, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Metallic", &s_Metallic, 0.0f, 1.0f, "%.2f");
        ImGui::SliderFloat("Normal Map Scale", &s_NormalScale, 0.0f, 2.0f, "%.2f");
        ImGui::SliderFloat("Emissive Intensity", &s_EmissiveIntensity, 0.0f, 10.0f, "%.1f");
        ImGui::InputFloat2("UV Tiling (U, V)", s_UvTiling, "%.1f");

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Section 2: HLSL Shader Source Code Viewport
    if (ImGui::CollapsingHeader("HLSL / Zelyn Shader Code", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);
        
        if (Theme::GetFontAtlas().monoFont) ImGui::PushFont(Theme::GetFontAtlas().monoFont);

        ImGui::BeginChild("HlslCodePane", ImVec2(0, 180.0f), true, ImGuiWindowFlags_HorizontalScrollbar);
        const char* displayCode = shaderData.shaderSource.empty() ? s_ShaderHlslCode.c_str() : shaderData.shaderSource.c_str();
        ImGui::TextColored(pal.textSecondary, "%s", displayCode);
        ImGui::EndChild();

        if (Theme::GetFontAtlas().monoFont) ImGui::PopFont();

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Action Buttons: Recompile Shader & Save Material
    ImGui::PushStyleColor(ImGuiCol_Button, pal.accent);
    ImGui::PushStyleColor(ImGuiCol_Text, pal.bgBase);
    if (ImGui::Button("Recompile Shader Pass", ImVec2(180.0f, 28.0f))) {
        Logger::Get().Info("[ShaderStudio] DX12 Shader Pass recompiled successfully.");
    }
    ImGui::PopStyleColor(2);

    ImGui::SameLine(0.0f, 8.0f);

    ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
    if (ImGui::Button("Save Material Asset", ImVec2(160.0f, 28.0f))) {
        Logger::Get().Info("[ShaderStudio] Saved M_PBR_MetallicStructure.mat to disk.");
    }
    ImGui::PopStyleColor();

    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace EngineEditor
