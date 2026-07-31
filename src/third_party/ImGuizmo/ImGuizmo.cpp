#include "ImGuizmo.h"
#include <cmath>

namespace ImGuizmo {

static ImDrawList* g_DrawList = nullptr;
static float g_X = 0, g_Y = 0, g_Width = 0, g_Height = 0;
static bool g_Enabled = true;
static bool g_IsUsing = false;
static bool g_IsOver = false;

void SetDrawlist(ImDrawList* drawlist) {
    g_DrawList = drawlist ? drawlist : ImGui::GetWindowDrawList();
}

void BeginFrame() {
    g_IsUsing = false;
    g_IsOver = false;
}

void SetRect(float x, float y, float width, float height) {
    g_X = x; g_Y = y; g_Width = width; g_Height = height;
}

void Enable(bool enable) {
    g_Enabled = enable;
}

bool IsUsing() {
    return g_IsUsing;
}

bool IsOver() {
    return g_IsOver;
}

void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matrix) {
    // Identity for stub matrix composition
    for (int i = 0; i < 16; ++i) matrix[i] = 0.0f;
    matrix[0] = scale[0];
    matrix[5] = scale[1];
    matrix[10] = scale[2];
    matrix[15] = 1.0f;
    matrix[12] = translation[0];
    matrix[13] = translation[1];
    matrix[14] = translation[2];
}

void DecomposeMatrixToComponents(const float* matrix, float* translation, float* rotation, float* scale) {
    translation[0] = matrix[12];
    translation[1] = matrix[13];
    translation[2] = matrix[14];
    scale[0] = matrix[0];
    scale[1] = matrix[5];
    scale[2] = matrix[10];
    rotation[0] = 0.0f;
    rotation[1] = 0.0f;
    rotation[2] = 0.0f;
}

void Manipulate(const float* view, const float* projection, OPERATION operation, MODE mode, float* matrix, float* deltaMatrix, const float* snap, const float* localBounds, const float* boundsSnap) {
    if (!g_Enabled) return;
    ImDrawList* drawList = g_DrawList ? g_DrawList : ImGui::GetWindowDrawList();
    if (!drawList) return;

    // Render 3D Axis Gizmo Overlay over object center
    ImVec2 center = ImVec2(g_X + g_Width * 0.5f, g_Y + g_Height * 0.5f);
    float size = 65.0f;

    // X Axis (Red)
    drawList->AddLine(center, ImVec2(center.x + size, center.y), IM_COL32(230, 60, 60, 255), 3.0f);
    drawList->AddTriangleFilled(ImVec2(center.x + size, center.y - 5), ImVec2(center.x + size + 10, center.y), ImVec2(center.x + size, center.y + 5), IM_COL32(230, 60, 60, 255));
    drawList->AddText(ImVec2(center.x + size + 12, center.y - 7), IM_COL32(230, 60, 60, 255), "X");

    // Y Axis (Green)
    drawList->AddLine(center, ImVec2(center.x, center.y - size), IM_COL32(60, 210, 60, 255), 3.0f);
    drawList->AddTriangleFilled(ImVec2(center.x - 5, center.y - size), ImVec2(center.x, center.y - size - 10), ImVec2(center.x + 5, center.y - size), IM_COL32(60, 210, 60, 255));
    drawList->AddText(ImVec2(center.x - 4, center.y - size - 22), IM_COL32(60, 210, 60, 255), "Y");

    // Z Axis (Blue)
    drawList->AddLine(center, ImVec2(center.x - size * 0.5f, center.y + size * 0.5f), IM_COL32(60, 130, 240, 255), 3.0f);
    drawList->AddText(ImVec2(center.x - size * 0.5f - 14, center.y + size * 0.5f), IM_COL32(60, 130, 240, 255), "Z");

    // Origin Square Hub
    drawList->AddCircleFilled(center, 5.0f, IM_COL32(240, 240, 240, 255));
}

void ViewManipulate(float* view, float length, ImVec2 position, ImVec2 size, ImU32 backgroundColor) {
    ImDrawList* drawList = g_DrawList ? g_DrawList : ImGui::GetWindowDrawList();
    if (!drawList) return;

    ImVec2 center = ImVec2(position.x + size.x * 0.5f, position.y + size.y * 0.5f);
    float half = size.x * 0.4f;

    // ViewCube Background Box
    drawList->AddRectFilled(ImVec2(center.x - half, center.y - half), ImVec2(center.x + half, center.y + half), backgroundColor, 4.0f);
    drawList->AddRect(ImVec2(center.x - half, center.y - half), ImVec2(center.x + half, center.y + half), IM_COL32(80, 160, 240, 255), 4.0f, 0, 1.5f);

    // Axis Handles
    drawList->AddText(ImVec2(center.x - 12.0f, center.y - 7.0f), IM_COL32(255, 255, 255, 255), "CUBE");
}

} // namespace ImGuizmo
