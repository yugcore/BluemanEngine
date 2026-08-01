#include "TextureViewerPanel.h"
#include "core/EditorState.h"
#include "core/Logger.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

static int s_ActiveChannelIndex = 0; // 0: RGB, 1: Red, 2: Green, 3: Blue, 4: Alpha
static int s_ActiveMipLevel = 0;

void RenderTextureViewerPanel(bool* pOpen) {
    if (!ImGui::Begin("Texture Viewer", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));

    // Texture Asset Header
    ImGui::TextColored(pal.textSecondary, "ACTIVE TEXTURE ASSET");
    ImGui::TextColored(pal.textPrimary, "T_Concrete_Normal_2K.png");
    ImGui::Separator();
    ImGui::Spacing();

    // RGBA Channel Selector Buttons
    ImGui::TextColored(pal.textSecondary, "Channel View:");
    const char* channels[] = { "RGB", "Red (R)", "Green (G)", "Blue (B)", "Alpha (A)" };
    for (int ch = 0; ch < 5; ++ch) {
        if (ch > 0) ImGui::SameLine(0.0f, 4.0f);
        bool isSel = (s_ActiveChannelIndex == ch);
        ImGui::PushStyleColor(ImGuiCol_Button, isSel ? pal.accent : pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_Text, isSel ? pal.bgBase : pal.textPrimary);
        if (ImGui::Button(channels[ch])) s_ActiveChannelIndex = ch;
        ImGui::PopStyleColor(2);
    }

    ImGui::Spacing();
    ImGui::SliderInt("Mipmap Tier", &s_ActiveMipLevel, 0, 10, "Mip %d (2048x2048)");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Texture Metadata Inspection Grid
    if (ImGui::CollapsingHeader("Texture Metadata & Compression", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent(8.0f);

        ImGui::TextColored(pal.textSecondary, "Resolution:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "2048 x 2048 px");

        ImGui::TextColored(pal.textSecondary, "Color Format:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "32-bit RGBA (8bpc)");

        ImGui::TextColored(pal.textSecondary, "GPU Compression:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "BC7_UNORM (DXGI_FORMAT_BC7_UNORM)");

        ImGui::TextColored(pal.textSecondary, "Total Memory:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "5.33 MB (with 11 Mipmaps)");

        ImGui::TextColored(pal.textSecondary, "Texture Type:");
        ImGui::SameLine(160.0f);
        ImGui::TextColored(pal.textPrimary, "Normal Map (Tangent Space)");

        ImGui::Unindent(8.0f);
    }

    ImGui::Spacing();

    // Canvas Preview Area
    ImGui::TextColored(pal.textSecondary, "Channel Preview Canvas:");
    ImVec2 canvasMin = ImGui::GetCursorScreenPos();
    float canvasWidth = ImGui::GetContentRegionAvail().x;
    float canvasHeight = 160.0f;

    // Reserve layout space in parent window
    ImGui::Dummy(ImVec2(canvasWidth, canvasHeight));

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Checkerboard Background Pattern
    drawList->AddRectFilled(canvasMin, ImVec2(canvasMin.x + canvasWidth, canvasMin.y + canvasHeight), IM_COL32(30, 30, 30, 255), 4.0f);
    drawList->AddRect(canvasMin, ImVec2(canvasMin.x + canvasWidth, canvasMin.y + canvasHeight), ImGui::ColorConvertFloat4ToU32(pal.borderSubtle), 4.0f);

    // Channel specific preview box
    ImVec2 previewMin = ImVec2(canvasMin.x + (canvasWidth - 120.0f) * 0.5f, canvasMin.y + (canvasHeight - 120.0f) * 0.5f);
    ImVec2 previewMax = ImVec2(previewMin.x + 120.0f, previewMin.y + 120.0f);

    ImU32 channelPreviewCol = IM_COL32(200, 200, 200, 255);
    if (s_ActiveChannelIndex == 1) channelPreviewCol = IM_COL32(230, 70, 70, 255);
    else if (s_ActiveChannelIndex == 2) channelPreviewCol = IM_COL32(70, 230, 70, 255);
    else if (s_ActiveChannelIndex == 3) channelPreviewCol = IM_COL32(70, 130, 250, 255);
    else if (s_ActiveChannelIndex == 4) channelPreviewCol = IM_COL32(180, 180, 180, 255);

    drawList->AddRectFilled(previewMin, previewMax, channelPreviewCol, 2.0f);

    ImGui::PopStyleVar();
    ImGui::End();
}

} // namespace EngineEditor
