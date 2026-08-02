#ifndef VIEWPORT_PICKER_H
#define VIEWPORT_PICKER_H

#include <string>
#include <vector>
#include <imgui.h>
#include "ViewportMath.h"
#include "engine/scene/SceneGraph.h"

namespace EngineEditor::Panels {

struct RaycastHit {
    bool hit = false;
    float distance = 1e9f;
    Vec3f point{ 0.0f, 0.0f, 0.0f };
    Vec3f normal{ 0.0f, 1.0f, 0.0f };
    std::string nodeName;
    uint64_t nodeId = 0;
};

class ViewportPicker {
public:
    static ViewportPicker& Get();

    // Raycasts into scene graph nodes to find closest intersected node
    bool PickNode(ImVec2 mousePos, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], RaycastHit& outHit);

    // Raycasts against infinite ground plane (Y=0) or surface geometry
    bool RaycastSurface(ImVec2 mousePos, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], RaycastHit& outHit);

    // Finds closest vertex on scene geometry near ray
    bool FindClosestVertex(const ViewportMath::Ray3D& ray, const float view[16], const float proj[16], ImVec2 cursorPos, ImVec2 viewportAvail, Vec3f& outVertexPos);

    // Computes world-space oriented AABB for a scene node including rotation and scale
    static AABB ComputeNodeWorldAABB(const SceneNode& node);

private:
    void RaycastNodeRecursive(const SceneNode& node, const ViewportMath::Ray3D& ray, RaycastHit& bestHit);
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_PICKER_H
