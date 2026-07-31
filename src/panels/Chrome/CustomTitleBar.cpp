#include "CustomTitleBar.h"
#include "MenuBar.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"
#include "theme/Fonts.h"

#include <imgui.h>
#include <imgui_internal.h>
#include <GLFW/glfw3.h>
#include <algorithm>

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

namespace EngineEditor {

static GLFWwindow* s_Window = nullptr;
static bool s_Dragging = false;
static double s_DragStartX = 0.0;
static double s_DragStartY = 0.0;
static int s_WinStartX = 0;
static int s_WinStartY = 0;

// Unified chrome metrics
static constexpr float kMinTitleBarHeight = 36.0f;
static constexpr float kFallbackFontSize  = 15.0f;
static constexpr float kBarVerticalPad    = 6.0f;
static constexpr float kWindowBtnWidth    = 42.0f;
static constexpr float kWindowBtnCount    = 3.0f;

static float s_TitleBarHeight = kMinTitleBarHeight;

void SetCustomTitleBarWindow(GLFWwindow* window) {
    s_Window = window;
}

float GetTitleBarTotalHeight() {
    return s_TitleBarHeight;
}

static bool RenderChromeButton(const char* id, ImVec2 size, ImVec4 hoverColor) {
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
    bool clicked = ImGui::Button(id, size);
    ImGui::PopStyleColor();
    return clicked;
}

static void DrawCloseGlyph(ImVec2 center, float sz, ImU32 col) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(center.x - sz, center.y - sz), ImVec2(center.x + sz, center.y + sz), col, 1.5f);
    dl->AddLine(ImVec2(center.x + sz, center.y - sz), ImVec2(center.x - sz, center.y + sz), col, 1.5f);
}

static void DrawMaximizeGlyph(ImVec2 center, float sz, ImU32 col) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddRect(ImVec2(center.x - sz, center.y - sz), ImVec2(center.x + sz, center.y + sz), col, 0.0f, 0, 1.5f);
}

static void DrawMinimizeGlyph(ImVec2 center, float sz, ImU32 col) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    dl->AddLine(ImVec2(center.x - sz, center.y), ImVec2(center.x + sz, center.y), col, 1.5f);
}

static ImVec2 GetLastItemCenter() {
    ImVec2 bMin = ImGui::GetItemRectMin();
    ImVec2 bMax = ImGui::GetItemRectMax();
    return ImVec2((bMin.x + bMax.x) * 0.5f, (bMin.y + bMax.y) * 0.5f);
}

static float ComputeTitleBarHeight() {
    float titleFontSize = kFallbackFontSize;
    if (Theme::GetFontAtlas().sectionHeaderFont)
        titleFontSize = Theme::GetFontAtlas().sectionHeaderFont->LegacySize;
    float h = titleFontSize + kBarVerticalPad * 2.0f;
    return std::max(h, kMinTitleBarHeight);
}

void RenderCustomTitleBar() {
    if (!s_Window) return;

    const auto& pal = Theme::GetPalette();
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    s_TitleBarHeight = ComputeTitleBarHeight();
    float framePadY = std::max((s_TitleBarHeight - kFallbackFontSize) * 0.5f, 4.0f);

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, s_TitleBarHeight));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
                             ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(Theme::Metrics::panelLeftMargin, framePadY));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgBase);
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, pal.bgBase);

    if (ImGui::Begin("##CustomTitleBar", nullptr, flags)) {
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        float btnZoneWidth = kWindowBtnWidth * kWindowBtnCount;

        float chromeBtnZoneX = windowSize.x - btnZoneWidth;
        ImVec2 mousePos = ImGui::GetMousePos();
        bool mouseInTitleBar = mousePos.x >= windowPos.x && mousePos.x <= windowPos.x + chromeBtnZoneX &&
                               mousePos.y >= windowPos.y && mousePos.y <= windowPos.y + windowSize.y;

        bool menuActive = ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId);
        if (mouseInTitleBar && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !menuActive) {
            s_Dragging = true;
            glfwGetCursorPos(s_Window, &s_DragStartX, &s_DragStartY);
            glfwGetWindowPos(s_Window, &s_WinStartX, &s_WinStartY);
        }
        if (s_Dragging) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                double curX, curY;
                glfwGetCursorPos(s_Window, &curX, &curY);
                double dx = curX - s_DragStartX;
                double dy = curY - s_DragStartY;
                glfwSetWindowPos(s_Window, s_WinStartX + (int)dx, s_WinStartY + (int)dy);
            } else {
                s_Dragging = false;
            }
        }

        if (mouseInTitleBar && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
            if (glfwGetWindowAttrib(s_Window, GLFW_MAXIMIZED)) {
                glfwRestoreWindow(s_Window);
            } else {
                glfwMaximizeWindow(s_Window);
            }
        }

        if (ImGui::BeginMenuBar()) {
            ImGui::SetCursorPosX(Theme::Metrics::panelLeftMargin);

            if (Theme::GetFontAtlas().sectionHeaderFont)
                ImGui::PushFont(Theme::GetFontAtlas().sectionHeaderFont);
            ImGui::TextColored(pal.accent, "BLUEMAN ENGINE");
            if (Theme::GetFontAtlas().sectionHeaderFont)
                ImGui::PopFont();

            ImGui::SameLine();
            ImGui::AlignTextToFramePadding();
            ImGui::TextColored(pal.textDisabled, "v2.0 Enterprise");
            ImGui::SameLine(0.0f, Theme::Metrics::groupGap);

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine(0.0f, Theme::Metrics::sectionIndent);

            RenderMenuBarContents();

            const char* workspaceLabel = "ZeGFX Workspace";
            float workspaceTagWidth = ImGui::CalcTextSize(workspaceLabel).x + 20.0f;
            float tagX = windowSize.x - btnZoneWidth - workspaceTagWidth;
            if (tagX > ImGui::GetCursorPosX()) {
                ImGui::SameLine(tagX);
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(pal.textSecondary, workspaceLabel);
            }

            float btnZoneX = std::max(windowSize.x - btnZoneWidth, ImGui::GetCursorPosX());
            ImGui::SameLine(btnZoneX);

            ImU32 glyphCol = IM_COL32(210, 210, 214, 255);
            float glyphSz = 5.0f;
            ImVec2 btnSize(kWindowBtnWidth, s_TitleBarHeight);

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.0f, 1.0f, 1.0f, 0.12f));

            if (RenderChromeButton("##Minimize", btnSize, ImVec4(1.0f, 1.0f, 1.0f, 0.08f))) {
                glfwIconifyWindow(s_Window);
            }
            DrawMinimizeGlyph(GetLastItemCenter(), glyphSz, glyphCol);

            ImGui::SameLine();
            if (RenderChromeButton("##MaxRestore", btnSize, ImVec4(1.0f, 1.0f, 1.0f, 0.08f))) {
                if (glfwGetWindowAttrib(s_Window, GLFW_MAXIMIZED)) {
                    glfwRestoreWindow(s_Window);
                } else {
                    glfwMaximizeWindow(s_Window);
                }
            }
            DrawMaximizeGlyph(GetLastItemCenter(), glyphSz, glyphCol);

            ImGui::SameLine();
            if (RenderChromeButton("##Close", btnSize, ImVec4(0.90f, 0.18f, 0.18f, 1.0f))) {
                glfwSetWindowShouldClose(s_Window, GLFW_TRUE);
            }
            DrawCloseGlyph(GetLastItemCenter(), glyphSz, glyphCol);

            ImGui::PopStyleColor(2);
            ImGui::PopStyleVar(3);

            ImGui::EndMenuBar();
        }
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor