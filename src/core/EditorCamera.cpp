#include "EditorCamera.h"
#include <windows.h>

namespace EngineEditor {

static constexpr float kPi = 3.14159265358979f;
static constexpr float kDegToRad = kPi / 180.0f;

static Vec3f Normalize(const Vec3f& v) {
    float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len > 0.0001f) {
        return Vec3f(v.x / len, v.y / len, v.z / len);
    }
    return Vec3f(0.0f, 0.0f, 0.0f);
}

static Vec3f Cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static float Dot(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

EditorCamera::EditorCamera() {
    UpdateVectors();
}

void EditorCamera::ResetToDefault() {
    m_Position = Vec3f(0.0f, 3.0f, -8.0f);
    m_Yaw = -90.0f;
    m_Pitch = -15.0f;
    m_MoveSpeed = 8.0f;
    m_OrbitPivot = Vec3f(0.0f, 0.0f, 0.0f);
    m_OrbitDistance = 8.0f;
    m_Mode = CameraMode::Idle;
    UpdateVectors();
}

void EditorCamera::UpdateVectors() {
    float yawRad = m_Yaw * kDegToRad;
    float pitchRad = m_Pitch * kDegToRad;

    Vec3f front;
    front.x = std::cos(yawRad) * std::cos(pitchRad);
    front.y = std::sin(pitchRad);
    front.z = std::sin(yawRad) * std::cos(pitchRad);
    m_Front = Normalize(front);

    // Left-handed convention (matches Mat4::lookAt and GetViewMatrix):
    // x = cross(worldUp, front), y = cross(front, x)
    m_Right = Normalize(Cross(m_WorldUp, m_Front));
    m_Up = Normalize(Cross(m_Front, m_Right));
}

void EditorCamera::Update(float deltaTime, const ViewportInputState& input) {
    if (m_SpeedTuningTimer > 0.0f) {
        m_SpeedTuningTimer -= deltaTime;
        if (m_SpeedTuningTimer <= 0.0f) {
            m_SpeedTuningActive = false;
        }
    }

    // Determine Camera Mode
    if (input.isHovered || input.isFocused || m_CursorLocked) {
        if (input.rmbDown && !input.altHeld) {
            m_Mode = CameraMode::Fly;
        } else if (input.lmbDown && input.altHeld) {
            m_Mode = CameraMode::Orbit;
        } else if (input.mmbDown) {
            m_Mode = CameraMode::Pan;
        } else {
            m_Mode = CameraMode::Idle;
        }
    } else {
        m_Mode = CameraMode::Idle;
    }

    // Handle OS Cursor Locking/Hiding
    if (m_Mode == CameraMode::Fly || m_Mode == CameraMode::Orbit || m_Mode == CameraMode::Pan) {
        if (!m_CursorLocked) {
            m_CursorLocked = true;
            POINT p;
            GetCursorPos(&p);
            m_SavedCursorPos = ImVec2((float)p.x, (float)p.y);
            ShowCursor(FALSE);
        }
    } else {
        if (m_CursorLocked) {
            m_CursorLocked = false;
            SetCursorPos((int)m_SavedCursorPos.x, (int)m_SavedCursorPos.y);
            ShowCursor(TRUE);
        }
    }

    // Mode 1: Fly Mode (RMB + WASDQE)
    if (m_Mode == CameraMode::Fly) {
        // Adjust Move Speed via Scroll Wheel
        if (std::abs(input.scrollDelta) > 0.01f) {
            m_MoveSpeed += input.scrollDelta * 1.5f;
            m_MoveSpeed = std::max(0.5f, std::min(m_MoveSpeed, 100.0f));
            m_SpeedTuningActive = true;
            m_SpeedTuningTimer = 2.0f;
        }

        // Mouse Look Rotation (Yaw & Pitch)
        if (input.mouseDeltaX != 0.0f || input.mouseDeltaY != 0.0f) {
            m_Yaw += input.mouseDeltaX * m_LookSensitivity;
            m_Pitch -= input.mouseDeltaY * m_LookSensitivity;
            m_Pitch = std::max(-89.0f, std::min(m_Pitch, 89.0f));
            UpdateVectors();
        }

        // WASDQE Keyboard Movement
        float speedMultiplier = 1.0f;
        if (input.shiftHeld) speedMultiplier = m_SpeedBoostMultiplier;
        else if (input.ctrlHeld) speedMultiplier = m_SlowSpeedMultiplier;

        float currentSpeed = m_MoveSpeed * speedMultiplier;
        Vec3f targetVel(0.0f, 0.0f, 0.0f);

        if (input.keyW) { targetVel.x += m_Front.x * currentSpeed; targetVel.y += m_Front.y * currentSpeed; targetVel.z += m_Front.z * currentSpeed; }
        if (input.keyS) { targetVel.x -= m_Front.x * currentSpeed; targetVel.y -= m_Front.y * currentSpeed; targetVel.z -= m_Front.z * currentSpeed; }
        if (input.keyD) { targetVel.x += m_Right.x * currentSpeed; targetVel.y += m_Right.y * currentSpeed; targetVel.z += m_Right.z * currentSpeed; }
        if (input.keyA) { targetVel.x -= m_Right.x * currentSpeed; targetVel.y -= m_Right.y * currentSpeed; targetVel.z -= m_Right.z * currentSpeed; }
        if (input.keyE) { targetVel.x += m_WorldUp.x * currentSpeed; targetVel.y += m_WorldUp.y * currentSpeed; targetVel.z += m_WorldUp.z * currentSpeed; }
        if (input.keyQ) { targetVel.x -= m_WorldUp.x * currentSpeed; targetVel.y -= m_WorldUp.y * currentSpeed; targetVel.z -= m_WorldUp.z * currentSpeed; }

        // Easing interpolation
        float lerpFactor = std::min(1.0f, deltaTime * 12.0f);
        m_CurrentVelocity.x += (targetVel.x - m_CurrentVelocity.x) * lerpFactor;
        m_CurrentVelocity.y += (targetVel.y - m_CurrentVelocity.y) * lerpFactor;
        m_CurrentVelocity.z += (targetVel.z - m_CurrentVelocity.z) * lerpFactor;

        m_Position.x += m_CurrentVelocity.x * deltaTime;
        m_Position.y += m_CurrentVelocity.y * deltaTime;
        m_Position.z += m_CurrentVelocity.z * deltaTime;
    }
    // Mode 2: Orbit Mode (Alt + LMB Drag)
    else if (m_Mode == CameraMode::Orbit) {
        if (input.mouseDeltaX != 0.0f || input.mouseDeltaY != 0.0f) {
            m_Yaw += input.mouseDeltaX * m_LookSensitivity;
            m_Pitch -= input.mouseDeltaY * m_LookSensitivity;
            m_Pitch = std::max(-89.0f, std::min(m_Pitch, 89.0f));
            UpdateVectors();

            m_Position.x = m_OrbitPivot.x - m_Front.x * m_OrbitDistance;
            m_Position.y = m_OrbitPivot.y - m_Front.y * m_OrbitDistance;
            m_Position.z = m_OrbitPivot.z - m_Front.z * m_OrbitDistance;
        }
    }
    // Mode 3: Pan Mode (MMB Drag)
    else if (m_Mode == CameraMode::Pan) {
        float panSpeed = m_MoveSpeed * 0.005f;
        m_Position.x -= (m_Right.x * input.mouseDeltaX - m_Up.x * input.mouseDeltaY) * panSpeed;
        m_Position.y -= (m_Right.y * input.mouseDeltaX - m_Up.y * input.mouseDeltaY) * panSpeed;
        m_Position.z -= (m_Right.z * input.mouseDeltaX - m_Up.z * input.mouseDeltaY) * panSpeed;
    }
    // Mode 4: Idle Dolly Scroll (Mouse wheel in viewport when not holding RMB)
    else if (m_Mode == CameraMode::Idle && input.isHovered && std::abs(input.scrollDelta) > 0.01f) {
        float dollyDist = input.scrollDelta * m_MoveSpeed * 0.2f;
        m_Position.x += m_Front.x * dollyDist;
        m_Position.y += m_Front.y * dollyDist;
        m_Position.z += m_Front.z * dollyDist;
    }
}

void EditorCamera::FrameSelection(const std::vector<AABB>& boundsList) {
    if (boundsList.empty()) {
        m_OrbitPivot = Vec3f(0.0f, 0.0f, 0.0f);
        m_OrbitDistance = 6.0f;
    } else {
        Vec3f minB = boundsList[0].minBounds;
        Vec3f maxB = boundsList[0].maxBounds;
        for (const auto& b : boundsList) {
            minB.x = std::min(minB.x, b.minBounds.x);
            minB.y = std::min(minB.y, b.minBounds.y);
            minB.z = std::min(minB.z, b.minBounds.z);

            maxB.x = std::max(maxB.x, b.maxBounds.x);
            maxB.y = std::max(maxB.y, b.maxBounds.y);
            maxB.z = std::max(maxB.z, b.maxBounds.z);
        }

        m_OrbitPivot = Vec3f(
            (minB.x + maxB.x) * 0.5f,
            (minB.y + maxB.y) * 0.5f,
            (minB.z + maxB.z) * 0.5f
        );

        float dx = maxB.x - minB.x;
        float dy = maxB.y - minB.y;
        float dz = maxB.z - minB.z;
        float radius = std::sqrt(dx * dx + dy * dy + dz * dz) * 0.5f;
        radius = std::max(1.5f, radius);

        m_OrbitDistance = radius / std::tan(m_Fov * 0.5f * kDegToRad) * 1.5f;
    }

    m_Position.x = m_OrbitPivot.x - m_Front.x * m_OrbitDistance;
    m_Position.y = m_OrbitPivot.y - m_Front.y * m_OrbitDistance;
    m_Position.z = m_OrbitPivot.z - m_Front.z * m_OrbitDistance;
}

void EditorCamera::GetViewMatrix(float outMatrix[16]) const {
    // Left-handed look-at (matches Mat4::lookAt convention)
    Vec3f target(m_Position.x + m_Front.x, m_Position.y + m_Front.y, m_Position.z + m_Front.z);
    Vec3f zAxis = Normalize(Vec3f(target.x - m_Position.x, target.y - m_Position.y, target.z - m_Position.z));
    Vec3f xAxis = Normalize(Cross(m_Up, zAxis));
    Vec3f yAxis = Cross(zAxis, xAxis);

    outMatrix[0]  = xAxis.x;
    outMatrix[1]  = yAxis.x;
    outMatrix[2]  = zAxis.x;
    outMatrix[3]  = 0.0f;

    outMatrix[4]  = xAxis.y;
    outMatrix[5]  = yAxis.y;
    outMatrix[6]  = zAxis.y;
    outMatrix[7]  = 0.0f;

    outMatrix[8]  = xAxis.z;
    outMatrix[9]  = yAxis.z;
    outMatrix[10] = zAxis.z;
    outMatrix[11] = 0.0f;

    outMatrix[12] = -Dot(xAxis, m_Position);
    outMatrix[13] = -Dot(yAxis, m_Position);
    outMatrix[14] = -Dot(zAxis, m_Position);
    outMatrix[15] = 1.0f;
}

void EditorCamera::SetPositionAndOrientation(const Vec3f& pos, float yaw, float pitch) {
    m_Position = pos;
    m_Yaw = yaw;
    m_Pitch = pitch;
    UpdateVectors();
}

void EditorCamera::SetProjectionMode(CameraProjectionMode mode) {
    m_ProjMode = mode;
    switch (mode) {
        case CameraProjectionMode::Top:
            m_Yaw = -90.0f; m_Pitch = -89.0f; break;
        case CameraProjectionMode::Bottom:
            m_Yaw = -90.0f; m_Pitch = 89.0f; break;
        case CameraProjectionMode::Front:
            m_Yaw = -90.0f; m_Pitch = 0.0f; break;
        case CameraProjectionMode::Back:
            m_Yaw = 90.0f; m_Pitch = 0.0f; break;
        case CameraProjectionMode::Left:
            m_Yaw = 0.0f; m_Pitch = 0.0f; break;
        case CameraProjectionMode::Right:
            m_Yaw = 180.0f; m_Pitch = 0.0f; break;
        case CameraProjectionMode::Perspective:
        default:
            break;
    }
    UpdateVectors();
}

void EditorCamera::SaveBookmark(int slot, const std::string& name) {
    if (slot < 0 || slot >= 10) return;
    m_Bookmarks[slot].isSet = true;
    m_Bookmarks[slot].name = name.empty() ? ("Bookmark " + std::to_string(slot)) : name;
    m_Bookmarks[slot].position = m_Position;
    m_Bookmarks[slot].yaw = m_Yaw;
    m_Bookmarks[slot].pitch = m_Pitch;
    m_Bookmarks[slot].fov = m_Fov;
}

bool EditorCamera::LoadBookmark(int slot) {
    if (slot < 0 || slot >= 10 || !m_Bookmarks[slot].isSet) return false;
    m_Position = m_Bookmarks[slot].position;
    m_Yaw = m_Bookmarks[slot].yaw;
    m_Pitch = m_Bookmarks[slot].pitch;
    m_Fov = m_Bookmarks[slot].fov;
    UpdateVectors();
    return true;
}

const CameraBookmark* EditorCamera::GetBookmark(int slot) const {
    if (slot < 0 || slot >= 10) return nullptr;
    return &m_Bookmarks[slot];
}

void EditorCamera::GetProjectionMatrix(float aspectRatio, float outMatrix[16]) const {
    for (int i = 0; i < 16; ++i) outMatrix[i] = 0.0f;

    if (m_ProjMode == CameraProjectionMode::Perspective) {
        // D3D12 / Left-Handed perspective projection matrix (Z range [0, 1])
        float fovRad = m_Fov * kDegToRad;
        float tanHalfFov = std::tan(fovRad / 2.0f);

        outMatrix[0]  = 1.0f / (aspectRatio * tanHalfFov);
        outMatrix[5]  = 1.0f / tanHalfFov;
        outMatrix[10] = m_FarPlane / (m_FarPlane - m_NearPlane);
        outMatrix[11] = 1.0f;
        outMatrix[14] = -(m_NearPlane * m_FarPlane) / (m_FarPlane - m_NearPlane);
    } else {
        // Orthographic projection matrix
        float orthoHeight = m_OrbitDistance * 1.5f;
        float orthoWidth = orthoHeight * aspectRatio;

        outMatrix[0]  = 2.0f / orthoWidth;
        outMatrix[5]  = 2.0f / orthoHeight;
        outMatrix[10] = 1.0f / (m_FarPlane - m_NearPlane);
        outMatrix[14] = -m_NearPlane / (m_FarPlane - m_NearPlane);
        outMatrix[15] = 1.0f;
    }
}

} // namespace EngineEditor
