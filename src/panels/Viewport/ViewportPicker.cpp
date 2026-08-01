#include "ViewportPicker.h"
#include <cmath>
#include <algorithm>

namespace EngineEditor::Panels {

ViewportPicker& ViewportPicker::Get() {
    static ViewportPicker instance;
    return instance;
}

AABB ViewportPicker::ComputeNodeWorldAABB(const SceneNode& node) {
    AABB box;
    // Standard unit cube bounds expanded by node scale & position
    float halfX = std::max(0.5f, std::abs(node.scale[0]) * 0.5f);
    float halfY = std::max(0.5f, std::abs(node.scale[1]) * 0.5f);
    float halfZ = std::max(0.5f, std::abs(node.scale[2]) * 0.5f);

    box.minBounds = Vec3f(node.location[0] - halfX, node.location[1] - halfY, node.location[2] - halfZ);
    box.maxBounds = Vec3f(node.location[0] + halfX, node.location[1] + halfY, node.location[2] + halfZ);
    return box;
}

void ViewportPicker::RaycastNodeRecursive(const SceneNode& node, const ViewportMath::Ray3D& ray, RaycastHit& bestHit) {
    if (node.type == SceneNodeType::Actor || node.type == SceneNodeType::Light || node.type == SceneNodeType::Camera || node.type == SceneNodeType::Terrain) {
        AABB worldBox = ComputeNodeWorldAABB(node);
        float dist = 0.0f;
        if (ViewportMath::RayIntersectsAABB(ray, worldBox, dist)) {
            if (dist < bestHit.distance && dist > 0.0f) {
                bestHit.hit = true;
                bestHit.distance = dist;
                bestHit.nodeName = node.name;
                bestHit.nodeId = node.id;
                bestHit.point = ViewportMath::Add(ray.origin, ViewportMath::Scale(ray.direction, dist));
                bestHit.normal = Vec3f(0.0f, 1.0f, 0.0f); // Default up normal
            }
        }
    }

    for (const auto& child : node.children) {
        RaycastNodeRecursive(child, ray, bestHit);
    }
}

bool ViewportPicker::PickNode(ImVec2 mousePos, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], RaycastHit& outHit) {
    ViewportMath::Ray3D ray = ViewportMath::ScreenToWorldRay(mousePos, cursorPos, viewportAvail, view, proj);
    
    RaycastHit bestHit;
    bestHit.hit = false;
    bestHit.distance = 1e9f;

    const auto& rootNodes = SceneGraph::Get().GetRootNodes();
    for (const auto& node : rootNodes) {
        RaycastNodeRecursive(node, ray, bestHit);
    }

    if (bestHit.hit) {
        outHit = bestHit;
        return true;
    }
    return false;
}

bool ViewportPicker::RaycastSurface(ImVec2 mousePos, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], RaycastHit& outHit) {
    ViewportMath::Ray3D ray = ViewportMath::ScreenToWorldRay(mousePos, cursorPos, viewportAvail, view, proj);

    // 1. First test scene objects
    RaycastHit objectHit;
    bool hitObject = PickNode(mousePos, cursorPos, viewportAvail, view, proj, objectHit);

    // 2. Test ground plane Y = 0
    float groundDist = 0.0f;
    Vec3f groundHitPoint;
    bool hitGround = ViewportMath::RayIntersectsPlane(ray, Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f), groundDist, groundHitPoint);

    if (hitObject && hitGround) {
        if (objectHit.distance < groundDist) {
            outHit = objectHit;
            return true;
        } else {
            outHit.hit = true;
            outHit.distance = groundDist;
            outHit.point = groundHitPoint;
            outHit.normal = Vec3f(0.0f, 1.0f, 0.0f);
            outHit.nodeName = "GroundPlane";
            outHit.nodeId = 0;
            return true;
        }
    } else if (hitObject) {
        outHit = objectHit;
        return true;
    } else if (hitGround) {
        outHit.hit = true;
        outHit.distance = groundDist;
        outHit.point = groundHitPoint;
        outHit.normal = Vec3f(0.0f, 1.0f, 0.0f);
        outHit.nodeName = "GroundPlane";
        outHit.nodeId = 0;
        return true;
    }

    return false;
}

bool ViewportPicker::FindClosestVertex(const ViewportMath::Ray3D& ray, const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail, Vec3f& outVertexPos) {
    RaycastHit hit;
    const auto& rootNodes = SceneGraph::Get().GetRootNodes();
    float bestDistSq = 1e9f;
    bool found = false;

    auto checkNodeVertices = [&](auto& self, const SceneNode& node) -> void {
        AABB box = ComputeNodeWorldAABB(node);
        Vec3f corners[8] = {
            box.minBounds,
            { box.maxBounds.x, box.minBounds.y, box.minBounds.z },
            { box.maxBounds.x, box.maxBounds.y, box.minBounds.z },
            { box.minBounds.x, box.maxBounds.y, box.minBounds.z },
            { box.minBounds.x, box.minBounds.y, box.maxBounds.z },
            { box.maxBounds.x, box.minBounds.y, box.maxBounds.z },
            box.maxBounds,
            { box.minBounds.x, box.maxBounds.y, box.maxBounds.z }
        };

        for (int i = 0; i < 8; ++i) {
            Vec3f toCorner = ViewportMath::Sub(corners[i], ray.origin);
            float projDist = ViewportMath::Dot(toCorner, ray.direction);
            if (projDist > 0.0f) {
                Vec3f closestOnRay = ViewportMath::Add(ray.origin, ViewportMath::Scale(ray.direction, projDist));
                float distSq = ViewportMath::LengthSq(ViewportMath::Sub(corners[i], closestOnRay));
                if (distSq < bestDistSq && distSq < 1.0f) { // Within 1m threshold
                    bestDistSq = distSq;
                    outVertexPos = corners[i];
                    found = true;
                }
            }
        }

        for (const auto& child : node.children) {
            self(self, child);
        }
    };

    for (const auto& root : rootNodes) {
        checkNodeVertices(checkNodeVertices, root);
    }

    return found;
}

} // namespace EngineEditor::Panels
