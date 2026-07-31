#ifndef IMGUIZMO_H
#define IMGUIZMO_H

#include <imgui.h>

namespace ImGuizmo {

enum OPERATION {
    TRANSLATE_X = (1u << 0),
    TRANSLATE_Y = (1u << 1),
    TRANSLATE_Z = (1u << 2),
    ROTATE_X    = (1u << 3),
    ROTATE_Y    = (1u << 4),
    ROTATE_Z    = (1u << 5),
    SCALE_X     = (1u << 6),
    SCALE_Y     = (1u << 7),
    SCALE_Z     = (1u << 8),
    TRANSLATE   = TRANSLATE_X | TRANSLATE_Y | TRANSLATE_Z,
    ROTATE      = ROTATE_X | ROTATE_Y | ROTATE_Z,
    SCALE       = SCALE_X | SCALE_Y | SCALE_Z,
    UNIVERSAL   = TRANSLATE | ROTATE | SCALE
};

enum MODE {
    LOCAL,
    WORLD
};

void SetDrawlist(ImDrawList* drawlist = nullptr);
void BeginFrame();
void SetRect(float x, float y, float width, float height);
void Enable(bool enable);

bool IsUsing();
bool IsOver();

void Manipulate(const float* view, const float* projection, OPERATION operation, MODE mode, float* matrix, float* deltaMatrix = nullptr, const float* snap = nullptr, const float* localBounds = nullptr, const float* boundsSnap = nullptr);

void ViewManipulate(float* view, float length, ImVec2 position, ImVec2 size, ImU32 backgroundColor);

void RecomposeMatrixFromComponents(const float* translation, const float* rotation, const float* scale, float* matrix);
void DecomposeMatrixToComponents(const float* matrix, float* translation, float* rotation, float* scale);

} // namespace ImGuizmo

#endif // IMGUIZMO_H
