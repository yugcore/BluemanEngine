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

static constexpr float kStatusBarHeight = 28.0f;

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

// Compact separator between status bar sections
static void StatusSeparator() {
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
    ImGui::TextDisabled("|");
    ImGui::SameLine(0.0f, Theme::Metrics::intraGroupGap);
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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(Theme::Metrics::panelLeftMargin, 4.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgBase);

    if (Theme::GetFontAtlas().secondaryFont) {
        ImGui::PushFont(Theme::GetFontAtlas().secondaryFont);
    }

    if (ImGui::Begin("##StatusBarWindow", nullptr, windowFlags)) {
        auto& state = EditorState::Get();
        const auto& stats = state.stats;
        const auto& settings = state.settings;

        // --- Quick-access panel buttons (left side) ---
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, pal.bgHeader);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pal.bgElevated);

        ImGui::Button("Content Browser", ImVec2(0, 20.0f));
        ImGui::SameLine();
        ImGui::Button("Asset Registry", ImVec2(0, 20.0f));

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        StatusSeparator();

        // --- Status dot + label ---
        ImVec4 statusColor = pal.textSecondary;
        const char* statusText = "Ready";
        if (state.status == EngineStatus::Building) {
            statusColor = pal.statusWarning;
            statusText = "Building";
        } else if (state.status == EngineStatus::Error) {
            statusColor = pal.statusError;
            statusText = "Error";
        }

        // Draw status dot
        ImVec2 dotPos = ImGui::GetCursorScreenPos();
        dotPos.y += 5.0f;
        ImGui::GetWindowDrawList()->AddCircleFilled(ImVec2(dotPos.x + 4.0f, dotPos.y + 4.0f), 4.0f,
            ImGui::ColorConvertFloat4ToU32(statusColor));
        ImGui::Dummy(ImVec2(12.0f, 0.0f));
        ImGui::SameLine();
        ImGui::TextColored(statusColor, "%s", statusText);

        StatusSeparator();

        // --- Performance stats ---
        ImGui::TextColored(pal.textSecondary, "FPS: %.1f", stats.fps);
        StatusSeparator();
        ImGui::TextColored(pal.textSecondary, "Frame: %.2f ms", stats.frameTimeMs);
        StatusSeparator();
        ImGui::TextColored(pal.textSecondary, "CPU: %.0f%%", state.cpuUsagePct);
        StatusSeparator();
        ImGui::TextColored(pal.textSecondary, "VRAM: %.1f / %.1f GB", stats.vramUsedGB, stats.vramTotalGB);

        StatusSeparator();

        // --- Scene stats ---
        ImGui::TextColored(pal.textSecondary, "Triangles: %s", FormatWithCommas(stats.triangleCount).c_str());
        StatusSeparator();
        ImGui::TextColored(pal.textSecondary, "Draw Calls: %s", FormatWithCommas(stats.drawCalls).c_str());
        StatusSeparator();
        ImGui::TextColored(pal.textSecondary, "Entities: %u", stats.entityCount);

        StatusSeparator();

        // --- Selection info ---
        const char* selectedName = state.selectedNodeName.empty() ? "None" : state.selectedNodeName.c_str();
        ImGui::TextColored(pal.accent, "Selected: %s", selectedName);

        StatusSeparator();

        // --- Engine / API info ---
        ImGui::TextColored(pal.textDisabled, "ZeGFX 3.5 Enterprise (dx12)");

        StatusSeparator();

        // --- Quality preset ---
        const char* presets[] = { "RTX Low", "RTX Medium", "RTX Ultra" };
        int pIdx = settings.qualityPreset;
        if (pIdx < 0) pIdx = 0; if (pIdx > 2) pIdx = 2;
        ImGui::TextColored(pal.accent, "%s", presets[pIdx]);

        StatusSeparator();

        // --- Editor mode ---
        ImGui::TextColored(pal.textDisabled, "%s", state.editorModeName.c_str());

        StatusSeparator();

        // --- Level name ---
        ImGui::TextColored(pal.textSecondary, "%s", state.currentLevelName.c_str());

        StatusSeparator();

        // --- Git branch ---
        ImGui::TextColored(pal.textDisabled, "Git: %s", state.gitBranch.c_str());
    }
    ImGui::End();

    if (Theme::GetFontAtlas().secondaryFont) {
        ImGui::PopFont();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor
