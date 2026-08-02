#include "OutputLogPanel.h"
#include "engine/core/Logger.h"
#include "core/EditorState.h"
#include "widgets/SearchBar.h"
#include "theme/Fonts.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "third_party/IconsFontAwesome6.h"

#include <imgui.h>
#include <algorithm>
#include <cctype>

namespace EngineEditor {

static ImVec4 GetLogSeverityColor(LogSeverity severity) {
    const auto& pal = Theme::GetPalette();
    switch (severity) {
        case LogSeverity::Info:    return pal.textPrimary;
        case LogSeverity::Warning: return pal.statusWarning;
        case LogSeverity::Error:   return pal.statusError;
        default:                   return pal.textSecondary;
    }
}

static char s_ConsoleFilter[128] = "";
static bool s_AutoScroll = true;
static bool s_ShowFilter = true;

static bool CaseInsensitiveContains(const std::string& str, const std::string& query) {
    if (query.empty()) return true;
    auto it = std::search(
        str.begin(), str.end(),
        query.begin(), query.end(),
        [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
    );
    return it != str.end();
}

void RenderOutputLogPanel(bool* pOpen) {
    bool* openPtr = pOpen ? pOpen : &EditorState::Get().settings.showOutputLog;
    if (!*openPtr) return;

    if (!ImGui::Begin("Output Log", openPtr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto& pal = Theme::GetPalette();

    if (ImGui::BeginTabBar("OutputLogTabBar")) {

        if (ImGui::BeginTabItem("Console")) {
            // Console toolbar
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

            if (ImGui::Button("Clear", ImVec2(50.0f, Theme::Metrics::rowHeight))) {
                Logger::Get().Clear();
            }
            ImGui::SameLine();
            ImGui::Checkbox("\xE2\x9C\x93 Filter", &s_ShowFilter);

            ImGui::SameLine(ImGui::GetWindowWidth() - 240.0f);
            Widgets::RenderSearchBar("##ConsoleFilter", s_ConsoleFilter, sizeof(s_ConsoleFilter), "Filter log output...", 230.0f);

            ImGui::PopStyleVar(2);
            
            ImGui::Spacing();

            // Console log output
            ImGui::BeginChild("ConsoleScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            if (Theme::GetFontAtlas().monoFont) {
                ImGui::PushFont(Theme::GetFontAtlas().monoFont);
            }

            std::string filterQuery = s_ConsoleFilter;
            const auto& messages = Logger::Get().GetMessages();

            for (const auto& log : messages) {
                if (!CaseInsensitiveContains(log.message, filterQuery)) {
                    continue;
                }

                ImVec4 color = GetLogSeverityColor(log.severity);

                ImGui::TextDisabled("[%s]", log.timestamp.c_str());
                ImGui::SameLine();

                const char* prefix = "[INFO]";
                if (log.severity == LogSeverity::Warning) prefix = "[WARN]";
                else if (log.severity == LogSeverity::Error) prefix = "[ERR!]";

                ImGui::TextColored(color, "%s %s", prefix, log.message.c_str());
            }

            if (Theme::GetFontAtlas().monoFont) {
                ImGui::PopFont();
            }

            if (s_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Logo")) {
            ImGui::Spacing();
            ImGui::TextDisabled("Engine Logo & Branding Assets");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Programs")) {
            ImGui::Spacing();
            ImGui::TextDisabled("Background Compilation & Bake Tasks");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Profiler")) {
            ImGui::Spacing();
            ImGui::TextDisabled("CPU / GPU Micro-Profiler Timeline");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Output")) {
            ImGui::Spacing();
            ImGui::TextDisabled("Build Output & Package Exporter Log");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

} // namespace EngineEditor
