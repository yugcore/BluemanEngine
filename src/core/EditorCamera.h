#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace EngineEditor {

enum class CameraMode {
    Idle,
    Fly,
    Orbit,
    Pan
};

struct ViewportInputState {
    bool isHovered = false;
    bool isFocused = false;
    bool rmbDown = false;
    bool lmbDown = false;
    bool mmbDown = false;
    bool altHeld = false;
    bool shiftHeld = false;
    bool ctrlHeld = false;
    float mouseDeltaX = 0.0f;
    float mouseDeltaY = 0.0f;
    float scrollDelta = 0.0f;
    bool keyW = false;
    bool keyA = false;
    bool keyS = false;
    bool keyD = false;
    bool keyQ = false;
    bool keyE = false;
};

struct Vec3f {
    float x = 0.0f, y = 0.0f, z = 0.0f;
    Vec3f() = default;
    Vec3f(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct AABB {
    Vec3f minBounds{ -1.0f, -1.0f, -1.0f };
    Vec3f maxBounds{  1.0f,  1.0f,  1.0f };
};

class EditorCamera {
public:
    EditorCamera();

    void Update(float deltaTime, const ViewportInputState& input);
    void FrameSelection(const std::vector<AABB>& boundsList);
    void ResetToDefault();

    void GetViewMatrix(float outMatrix[16]) const;
    void GetProjectionMatrix(float aspectRatio, float outMatrix[16]) const;

    CameraMode GetMode() const { return m_Mode; }
    float GetMoveSpeed() const { return m_MoveSpeed; }
    Vec3f GetPosition() const { return m_Position; }
    bool IsSpeedTuningActive() const { return m_SpeedTuningActive; }

private:
    void UpdateVectors();

    CameraMode m_Mode = CameraMode::Idle;

    Vec3f m_Position{ 0.0f, 3.0f, -8.0f };
    Vec3f m_Front{ 0.0f, -0.3f, 0.95f };
    Vec3f m_Up{ 0.0f, 1.0f, 0.0f };
    Vec3f m_Right{ 1.0f, 0.0f, 0.0f };
    Vec3f m_WorldUp{ 0.0f, 1.0f, 0.0f };

    Vec3f m_OrbitPivot{ 0.0f, 0.0f, 0.0f };
    float m_OrbitDistance = 8.0f;

    float m_Yaw = -90.0f;
    float m_Pitch = -15.0f;

    float m_MoveSpeed = 8.0f;
    float m_LookSensitivity = 0.15f;
    float m_Fov = 60.0f;
    float m_NearPlane = 0.1f;
    float m_FarPlane = 1000.0f;
    float m_SpeedBoostMultiplier = 2.5f;

    Vec3f m_CurrentVelocity{ 0.0f, 0.0f, 0.0f };
    bool m_SpeedTuningActive = false;
    float m_SpeedTuningTimer = 0.0f;

    bool m_CursorLocked = false;
    ImVec2 m_SavedCursorPos{ 0.0f, 0.0f };
};

} // namespace EngineEditor

#endif // EDITOR_CAMERA_H
