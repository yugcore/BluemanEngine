#ifndef VIEWPORT_MATH_H
#define VIEWPORT_MATH_H

#include <cmath>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <imgui.h>
#include "core/EditorCamera.h" // For Vec3f, AABB

namespace EngineEditor::ViewportMath {

static constexpr float kPi = 3.14159265358979323846f;
static constexpr float kDegToRad = kPi / 180.0f;
static constexpr float kRadToDeg = 180.0f / kPi;

struct Ray3D {
    Vec3f origin;
    Vec3f direction; // Normalized
};

struct FrustumPlane {
    Vec3f normal;
    float distance = 0.0f;
};

struct Frustum3D {
    FrustumPlane planes[6]; // Near, Far, Left, Right, Top, Bottom
};

// --- Basic Vector Math Helpers ---

inline Vec3f Add(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vec3f Sub(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vec3f Scale(const Vec3f& v, float s) {
    return Vec3f(v.x * s, v.y * s, v.z * s);
}

inline float Dot(const Vec3f& a, const Vec3f& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3f Cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline float Length(const Vec3f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float LengthSq(const Vec3f& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline Vec3f Normalize(const Vec3f& v) {
    float len = Length(v);
    if (len > 0.00001f) {
        return Vec3f(v.x / len, v.y / len, v.z / len);
    }
    return Vec3f(0.0f, 0.0f, 0.0f);
}

inline Vec3f Lerp(const Vec3f& a, const Vec3f& b, float t) {
    return Vec3f(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

inline float Distance(const Vec3f& a, const Vec3f& b) {
    return Length(Sub(a, b));
}

// --- Matrix Inversion (4x4) ---
inline bool InvertMatrix4x4(const float m[16], float invOut[16]) {
    float inv[16];
    inv[0] = m[5]  * m[10] * m[15] - m[5]  * m[11] * m[14] - m[9]  * m[6]  * m[15] + m[9]  * m[7]  * m[14] + m[13] * m[6]  * m[11] - m[13] * m[7]  * m[10];
    inv[4] = -m[4]  * m[10] * m[15] + m[4]  * m[11] * m[14] + m[8]  * m[6]  * m[15] - m[8]  * m[7]  * m[14] - m[12] * m[6]  * m[11] + m[12] * m[7]  * m[10];
    inv[8] = m[4]  * m[9]  * m[15] - m[4]  * m[11] * m[13] - m[8]  * m[5]  * m[15] + m[8]  * m[7]  * m[13] + m[12] * m[5]  * m[11] - m[12] * m[7]  * m[9];
    inv[12] = -m[4]  * m[9]  * m[14] + m[4]  * m[10] * m[13] + m[8]  * m[5]  * m[14] - m[8]  * m[6]  * m[13] - m[12] * m[5]  * m[10] + m[12] * m[6]  * m[9];
    inv[1] = -m[1]  * m[10] * m[15] + m[1]  * m[11] * m[14] + m[9]  * m[2]  * m[15] - m[9]  * m[3]  * m[14] - m[13] * m[2]  * m[11] + m[13] * m[3]  * m[10];
    inv[5] = m[0]  * m[10] * m[15] - m[0]  * m[11] * m[14] - m[8]  * m[2]  * m[15] + m[8]  * m[3]  * m[14] + m[12] * m[2]  * m[11] - m[12] * m[3]  * m[10];
    inv[9] = -m[0]  * m[9]  * m[15] + m[0]  * m[11] * m[13] + m[8]  * m[1]  * m[15] - m[8]  * m[3]  * m[13] - m[12] * m[1]  * m[11] + m[12] * m[3]  * m[9];
    inv[13] = m[0]  * m[9]  * m[14] - m[0]  * m[10] * m[13] - m[8]  * m[1]  * m[14] + m[8]  * m[2]  * m[13] + m[12] * m[1]  * m[10] - m[12] * m[2]  * m[9];
    inv[2] = m[1]  * m[6]  * m[15] - m[1]  * m[7]  * m[14] - m[5]  * m[2]  * m[15] + m[5]  * m[3]  * m[14] + m[13] * m[2]  * m[7]  - m[13] * m[3]  * m[6];
    inv[6] = -m[0]  * m[6]  * m[15] + m[0]  * m[7]  * m[14] + m[4]  * m[2]  * m[15] - m[4]  * m[3]  * m[14] - m[12] * m[2]  * m[7]  + m[12] * m[3]  * m[6];
    inv[10] = m[0]  * m[5]  * m[15] - m[0]  * m[7]  * m[13] - m[4]  * m[1]  * m[15] + m[4]  * m[3]  * m[13] + m[12] * m[1]  * m[7]  - m[12] * m[3]  * m[5];
    inv[14] = -m[0]  * m[5]  * m[14] + m[0]  * m[6]  * m[13] + m[4]  * m[1]  * m[14] - m[4]  * m[2]  * m[13] - m[12] * m[1]  * m[6]  + m[12] * m[2]  * m[5];
    inv[3] = -m[1]  * m[6]  * m[11] + m[1]  * m[7]  * m[10] + m[5]  * m[2]  * m[11] - m[5]  * m[3]  * m[10] - m[9]  * m[2]  * m[7]  + m[9]  * m[3]  * m[6];
    inv[7] = m[0]  * m[6]  * m[11] - m[0]  * m[7]  * m[10] - m[4]  * m[2]  * m[11] + m[4]  * m[3]  * m[10] + m[8]  * m[2]  * m[7]  - m[8]  * m[3]  * m[6];
    inv[11] = -m[0]  * m[5]  * m[11] + m[0]  * m[7]  * m[9]  + m[4]  * m[1]  * m[11] - m[4]  * m[3]  * m[9]  - m[8]  * m[1]  * m[7]  + m[8]  * m[3]  * m[5];
    inv[15] = m[0]  * m[5]  * m[10] - m[0]  * m[6]  * m[9]  - m[4]  * m[1]  * m[10] + m[4]  * m[2]  * m[9]  + m[8]  * m[1]  * m[6]  - m[8]  * m[2]  * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];
    if (std::abs(det) < 1e-6f) return false;

    float invDet = 1.0f / det;
    for (int i = 0; i < 16; i++) invOut[i] = inv[i] * invDet;
    return true;
}

inline void MultiplyMatrix4x4(const float a[16], const float b[16], float outMat[16]) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            outMat[r * 4 + c] = 
                a[r * 4 + 0] * b[0 * 4 + c] +
                a[r * 4 + 1] * b[1 * 4 + c] +
                a[r * 4 + 2] * b[2 * 4 + c] +
                a[r * 4 + 3] * b[3 * 4 + c];
        }
    }
}

// --- World / Screen Projection Helpers ---

inline bool WorldToScreen(const Vec3f& pos, const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail, ImVec2& outScreen) {
    float x = pos.x, y = pos.y, z = pos.z;

    float clipX = x * view[0] + y * view[4] + z * view[8] + view[12];
    float clipY = x * view[1] + y * view[5] + z * view[9] + view[13];
    float clipZ = x * view[2] + y * view[6] + z * view[10] + view[14];
    float clipW = x * view[3] + y * view[7] + z * view[11] + view[15];

    float ndcX = clipX * proj[0] + clipY * proj[4] + clipZ * proj[8] + proj[12];
    float ndcY = clipX * proj[1] + clipY * proj[5] + clipZ * proj[9] + proj[13];
    float ndcW = clipX * proj[3] + clipY * proj[7] + clipZ * proj[11] + proj[15];

    if (ndcW <= 0.001f) return false;

    float ndc2X = ndcX / ndcW;
    float ndc2Y = ndcY / ndcW;

    outScreen.x = cursorPos.x + (ndc2X * 0.5f + 0.5f) * viewportAvail.x;
    outScreen.y = cursorPos.y + (1.0f - (ndc2Y * 0.5f + 0.5f)) * viewportAvail.y;

    return true;
}

// 3D Ray Generation from mouse cursor
inline Ray3D ScreenToWorldRay(ImVec2 mousePos, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]) {
    Ray3D ray;
    if (viewportAvail.x <= 0.0f || viewportAvail.y <= 0.0f) return ray;

    // Relative viewport screen coordinates [-1.0, 1.0]
    float relX = ((mousePos.x - cursorPos.x) / viewportAvail.x) * 2.0f - 1.0f;
    float relY = 1.0f - ((mousePos.y - cursorPos.y) / viewportAvail.y) * 2.0f;

    float viewProj[16];
    MultiplyMatrix4x4(view, proj, viewProj);

    float invViewProj[16];
    if (!InvertMatrix4x4(viewProj, invViewProj)) {
        return ray;
    }

    // Near point in NDC (Z = 0.0 in DX12)
    float nearPointNDC[4] = { relX, relY, 0.0f, 1.0f };
    float farPointNDC[4]  = { relX, relY, 1.0f, 1.0f };

    auto transformVec = [](const float v[4], const float m[16], Vec3f& outV) {
        float x = v[0] * m[0] + v[1] * m[4] + v[2] * m[8]  + v[3] * m[12];
        float y = v[0] * m[1] + v[1] * m[5] + v[2] * m[9]  + v[3] * m[13];
        float z = v[0] * m[2] + v[1] * m[6] + v[2] * m[10] + v[3] * m[14];
        float w = v[0] * m[3] + v[1] * m[7] + v[2] * m[11] + v[3] * m[15];
        if (std::abs(w) > 1e-6f) {
            outV = Vec3f(x / w, y / w, z / w);
        }
    };

    Vec3f nearWorld, farWorld;
    transformVec(nearPointNDC, invViewProj, nearWorld);
    transformVec(farPointNDC, invViewProj, farWorld);

    ray.origin = nearWorld;
    ray.direction = Normalize(Sub(farWorld, nearWorld));

    return ray;
}

// --- Ray vs Geometry Intersections ---

inline bool RayIntersectsAABB(const Ray3D& ray, const AABB& box, float& outDist) {
    float tMin = (box.minBounds.x - ray.origin.x) / (std::abs(ray.direction.x) > 1e-6f ? ray.direction.x : (ray.direction.x >= 0 ? 1e-6f : -1e-6f));
    float tMax = (box.maxBounds.x - ray.origin.x) / (std::abs(ray.direction.x) > 1e-6f ? ray.direction.x : (ray.direction.x >= 0 ? 1e-6f : -1e-6f));
    if (tMin > tMax) std::swap(tMin, tMax);

    float tyMin = (box.minBounds.y - ray.origin.y) / (std::abs(ray.direction.y) > 1e-6f ? ray.direction.y : (ray.direction.y >= 0 ? 1e-6f : -1e-6f));
    float tyMax = (box.maxBounds.y - ray.origin.y) / (std::abs(ray.direction.y) > 1e-6f ? ray.direction.y : (ray.direction.y >= 0 ? 1e-6f : -1e-6f));
    if (tyMin > tyMax) std::swap(tyMin, tyMax);

    if ((tMin > tyMax) || (tyMin > tMax)) return false;
    if (tyMin > tMin) tMin = tyMin;
    if (tyMax < tMax) tMax = tyMax;

    float tzMin = (box.minBounds.z - ray.origin.z) / (std::abs(ray.direction.z) > 1e-6f ? ray.direction.z : (ray.direction.z >= 0 ? 1e-6f : -1e-6f));
    float tzMax = (box.maxBounds.z - ray.origin.z) / (std::abs(ray.direction.z) > 1e-6f ? ray.direction.z : (ray.direction.z >= 0 ? 1e-6f : -1e-6f));
    if (tzMin > tzMax) std::swap(tzMin, tzMax);

    if ((tMin > tzMax) || (tzMin > tMax)) return false;
    if (tzMin > tMin) tMin = tzMin;
    if (tzMax < tMax) tMax = tzMax;

    if (tMax < 0.0f) return false;

    outDist = (tMin >= 0.0f) ? tMin : tMax;
    return true;
}

inline bool RayIntersectsPlane(const Ray3D& ray, const Vec3f& planePoint, const Vec3f& planeNormal, float& outDist, Vec3f& outHitPoint) {
    float denom = Dot(planeNormal, ray.direction);
    if (std::abs(denom) < 1e-6f) return false;

    float t = Dot(Sub(planePoint, ray.origin), planeNormal) / denom;
    if (t < 0.0f) return false;

    outDist = t;
    outHitPoint = Add(ray.origin, Scale(ray.direction, t));
    return true;
}

// Check if 2D screen box intersects projected 3D AABB
inline bool ScreenRectIntersectsAABB(ImVec2 rectMin, ImVec2 rectMax, const AABB& box, const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail) {
    Vec3f corners[8] = {
        { box.minBounds.x, box.minBounds.y, box.minBounds.z },
        { box.maxBounds.x, box.minBounds.y, box.minBounds.z },
        { box.maxBounds.x, box.maxBounds.y, box.minBounds.z },
        { box.minBounds.x, box.maxBounds.y, box.minBounds.z },
        { box.minBounds.x, box.minBounds.y, box.maxBounds.z },
        { box.maxBounds.x, box.minBounds.y, box.maxBounds.z },
        { box.maxBounds.x, box.maxBounds.y, box.maxBounds.z },
        { box.minBounds.x, box.maxBounds.y, box.maxBounds.z }
    };

    float minX = 1e9f, maxX = -1e9f;
    float minY = 1e9f, maxY = -1e9f;
    bool anyValid = false;

    for (int i = 0; i < 8; ++i) {
        ImVec2 scr;
        if (WorldToScreen(corners[i], view, proj, cursorPos, viewportAvail, scr)) {
            anyValid = true;
            minX = std::min(minX, scr.x);
            maxX = std::max(maxX, scr.x);
            minY = std::min(minY, scr.y);
            maxY = std::max(maxY, scr.y);
        }
    }

    if (!anyValid) return false;

    // Check 2D bounding box overlap
    float rMinX = std::min(rectMin.x, rectMax.x);
    float rMaxX = std::max(rectMin.x, rectMax.x);
    float rMinY = std::min(rectMin.y, rectMax.y);
    float rMaxY = std::max(rectMin.y, rectMax.y);

    return (minX <= rMaxX && maxX >= rMinX && minY <= rMaxY && maxY >= rMinY);
}

} // namespace EngineEditor::ViewportMath

#endif // VIEWPORT_MATH_H
