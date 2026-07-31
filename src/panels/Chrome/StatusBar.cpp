#include "StatusBar.h"
#include "core/EditorState.h"
#include "theme/Fonts.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <cstdint>
#include <cstdio>
#include <string>

namespace EngineEditor {

static constexpr float kStatusBarHeight = 32.0f;

float GetStatusBarTotalHeight() {
    return kStatusBarHeight;
}

static std::string FormatWithCommas(uint32_t value) {
    std::string numStr = std::to_string(value);
    int n = (int)numStr.length() - 3;
    while (n > 0) {
        numStr.insert(n, ",");
        n -= 3;
    }
    return numStr;
}

static void RenderVerticalSeparator(const Theme::Palette& pal, float windowTopY) {
    ImGui::SameLine(0.0f, 12.0f);
    ImVec2 p = ImGui::GetCursorScreenPos();
    float lineY1 = windowTopY + (kStatusBarHeight - 14.0f) * 0.5f;
    float lineY2 = lineY1 + 14.0f;
    ImU32 col = ImGui::ColorConvertFloat4ToU32(pal.borderSubtle);
    ImGui::GetWindowDrawList()->AddLine(ImVec2(p.x, lineY1), ImVec2(p.x, lineY2), col, 1.0f);
    ImGui::Dummy(ImVec2(1.0f, 0.0f));
    ImGui::SameLine(0.0f, 12.0f);
}

static void RenderStatItem(const char* label, const char* value, const Theme::Palette& pal, float textY) {
    ImGui::SetCursorPosY(textY);
    if (label && label[0] != '\0') {
        ImGui::TextColored(pal.textSecondary, "%s", label);
        ImGui::SameLine(0.0f, 4.0f);
    }
    ImGui::TextColored(pal.textPrimary, "%s", value);
}

void RenderStatusBar() {
    const auto& pal = Theme::GetPalette();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - kStatusBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, kStatusBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                   ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking |
                                   ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgBase);

    bool fontPushed = false;
    if (Theme::GetFontAtlas().monoFont) {
        ImGui::PushFont(Theme::GetFontAtlas().monoFont);
        fontPushed = true;
    }

    if (ImGui::Begin("##StatusBarWindow", nullptr, windowFlags)) {
        auto& state = EditorState::Get();
        const auto& stats = state.stats;
        auto& settings = state.settings;

        float windowTopY = ImGui::GetWindowPos().y;
        float textY = (kStatusBarHeight - ImGui::GetTextLineHeight()) * 0.5f;

        // ====================================================================
        // LEFT SIDE: Quick Access Panel Tab Pills (Content Browser & Output Log)
        // ====================================================================
        bool cbActive = settings.showContentBrowser;
        ImVec4 cbTextColor = cbActive ? pal.accent : pal.textSecondary;
        ImVec4 cbBgColor = cbActive ? ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.18f) : ImVec4(0, 0, 0, 0);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, cbBgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, pal.bgElevated);
        ImGui::PushStyleColor(ImGuiCol_Text, cbTextColor);

        float buttonY = (kStatusBarHeight - 22.0f) * 0.5f;
        ImGui::SetCursorPosY(buttonY);
        if (ImGui::Button("Content Browser", ImVec2(0.0f, 22.0f))) {
            settings.showContentBrowser = !settings.showContentBrowser;
        }

        // Output Log Tab Pill
        bool logActive = settings.showOutputLog;
        ImVec4 logTextColor = logActive ? pal.accent : pal.textSecondary;
        ImVec4 logBgColor = logActive ? ImVec4(pal.accent.x, pal.accent.y, pal.accent.z, 0.18f) : ImVec4(0, 0, 0, 0);

        ImGui::SameLine(0.0f, 6.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, logBgColor);
        ImGui::PushStyleColor(ImGuiCol_Text, logTextColor);
        if (ImGui::Button("Output Log", ImVec2(0.0f, 22.0f))) {
            settings.showOutputLog = !settings.showOutputLog;
        }
        ImGui::PopStyleColor(2);

        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar(2);

        float leftGroupEndX = ImGui::GetCursorPosX();

        // ====================================================================
        // RIGHT SIDE: All Metrics & Status Info (Right-Aligned Group)
        // ====================================================================
        const float dividerGap = 25.0f; // 12px left + 1px divider + 12px right

        // 1. Asset Registry
        const char* arText = "Asset Registry";

        // 2. Engine Status
        ImVec4 statusDotColor = ImVec4(0.2f, 0.8f, 0.4f, 1.0f); // Ready Green
        const char* statusText = "Ready";
        if (state.status == EngineStatus::Building) {
            statusDotColor = pal.statusWarning;
            statusText = "Building...";
        } else if (state.status == EngineStatus::Error) {
            statusDotColor = pal.statusError;
            statusText = "Error";
        }

        // 3. GPU
        char gpuVal[64];
        snprintf(gpuVal, sizeof(gpuVal), "%s | RTX 4080", stats.apiTag.c_str());

        // 4. FPS
        char fpsVal[64];
        snprintf(fpsVal, sizeof(fpsVal), "%.0f (%.1f ms)", stats.fps, stats.frameTimeMs);

        // 5. VRAM
        char vramVal[32];
        snprintf(vramVal, sizeof(vramVal), "%.1fGB", stats.vramUsedGB);

        // 6. RAM
        char ramVal[32];
        snprintf(ramVal, sizeof(ramVal), "%.1fGB", stats.ramUsedGB);

        // 7. Draws
        std::string drawVal = FormatWithCommas(stats.drawCalls);

        // 8. Entities
        char entVal[32];
        snprintf(entVal, sizeof(entVal), "%u", stats.entityCount);

        // 9. Quality Preset
        const char* presets[] = { "RTX Low", "RTX Medium", "RTX Ultra" };
        int pIdx = settings.qualityPreset;
        if (pIdx < 0) pIdx = 0; if (pIdx > 2) pIdx = 2;

        // 10. Git Branch
        const char* gitVal = state.gitBranch.c_str();

        // Calculate total width of the right group
        float rightGroupWidth = 0.0f;
        rightGroupWidth += ImGui::CalcTextSize(arText).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("\xE2\x97\x8F").x + 5.0f + ImGui::CalcTextSize(statusText).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("GPU: ").x + ImGui::CalcTextSize(gpuVal).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("FPS: ").x + ImGui::CalcTextSize(fpsVal).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("VRAM: ").x + ImGui::CalcTextSize(vramVal).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("RAM: ").x + ImGui::CalcTextSize(ramVal).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("Draws: ").x + ImGui::CalcTextSize(drawVal.c_str()).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("Entities: ").x + ImGui::CalcTextSize(entVal).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("Preset: ").x + ImGui::CalcTextSize(presets[pIdx]).x + dividerGap;
        rightGroupWidth += ImGui::CalcTextSize("Git: ").x + ImGui::CalcTextSize(gitVal).x;

        float winWidth = ImGui::GetWindowWidth();
        float rightStartX = winWidth - rightGroupWidth - 12.0f; // 12px window padding
        if (rightStartX < leftGroupEndX + 16.0f) {
            rightStartX = leftGroupEndX + 16.0f;
        }

        ImGui::SameLine(rightStartX);

        // 1. Asset Registry
        ImGui::SetCursorPosY(textY);
        ImGui::TextColored(pal.textSecondary, "%s", arText);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 2. Engine Status
        ImGui::SetCursorPosY(textY);
        ImGui::TextColored(statusDotColor, "\xE2\x97\x8F");
        ImGui::SameLine(0.0f, 5.0f);
        ImGui::TextColored(pal.textPrimary, "%s", statusText);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 3. GPU / API
        RenderStatItem("GPU:", gpuVal, pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 4. FPS & Frame Time
        RenderStatItem("FPS:", fpsVal, pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 5. VRAM
        RenderStatItem("VRAM:", vramVal, pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 6. RAM
        RenderStatItem("RAM:", ramVal, pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 7. Draws
        RenderStatItem("Draws:", drawVal.c_str(), pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 8. Entities
        RenderStatItem("Entities:", entVal, pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 9. Quality Preset
        RenderStatItem("Preset:", presets[pIdx], pal, textY);

        // Separator
        RenderVerticalSeparator(pal, windowTopY);

        // 10. Git Branch
        RenderStatItem("Git:", gitVal, pal, textY);
    }
    ImGui::End();

    if (fontPushed) {
        ImGui::PopFont();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

} // namespace EngineEditor
