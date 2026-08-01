#ifndef EDITOR_CAMERA_H
#define EDITOR_CAMERA_H

#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>

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

enum class CameraProjectionMode {
    Perspective,
    Top,
    Bottom,
    Front,
    Back,
    Left,
    Right
};

struct CameraBookmark {
    bool isSet = false;
    std::string name;
    Vec3f position{ 0.0f, 3.0f, -8.0f };
    float yaw = -90.0f;
    float pitch = -15.0f;
    float fov = 60.0f;
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
    void SetMoveSpeed(float speed) { m_MoveSpeed = std::max(0.1f, std::min(speed, 200.0f)); }

    Vec3f GetPosition() const { return m_Position; }
    float GetYaw() const { return m_Yaw; }
    float GetPitch() const { return m_Pitch; }
    float GetFov() const { return m_Fov; }
    void SetFov(float fov) { m_Fov = std::max(10.0f, std::min(fov, 140.0f)); }

    float GetNearPlane() const { return m_NearPlane; }
    void SetNearPlane(float nearP) { m_NearPlane = std::max(0.01f, nearP); }
    float GetFarPlane() const { return m_FarPlane; }
    void SetFarPlane(float farP) { m_FarPlane = std::max(10.0f, farP); }

    CameraProjectionMode GetProjectionMode() const { return m_ProjMode; }
    void SetProjectionMode(CameraProjectionMode mode);

    bool IsSpeedTuningActive() const { return m_SpeedTuningActive; }

    void SetPositionAndOrientation(const Vec3f& pos, float yaw, float pitch);
    
    // Bookmarks (10 slots)
    void SaveBookmark(int slot, const std::string& name = "");
    bool LoadBookmark(int slot);
    const CameraBookmark* GetBookmark(int slot) const;

private:
    void UpdateVectors();

    CameraMode m_Mode = CameraMode::Idle;
    CameraProjectionMode m_ProjMode = CameraProjectionMode::Perspective;

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
    float m_SlowSpeedMultiplier = 0.25f;

    Vec3f m_CurrentVelocity{ 0.0f, 0.0f, 0.0f };
    bool m_SpeedTuningActive = false;
    float m_SpeedTuningTimer = 0.0f;

    // Smooth focus animation target state
    bool m_IsFramingSmooth = false;
    Vec3f m_FrameTargetPos{ 0.0f, 0.0f, 0.0f };
    Vec3f m_FrameTargetPivot{ 0.0f, 0.0f, 0.0f };
    float m_FrameAnimTime = 0.0f;

    bool m_CursorLocked = false;
    ImVec2 m_SavedCursorPos{ 0.0f, 0.0f };

    CameraBookmark m_Bookmarks[10];
};

} // namespace EngineEditor

#endif // EDITOR_CAMERA_H
