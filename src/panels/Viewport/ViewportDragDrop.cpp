#include "ViewportDragDrop.h"
#include "ViewportPicker.h"
#include "ViewportSelection.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/Logger.h"
#include <filesystem>

namespace EngineEditor::Panels {

ViewportDragDrop& ViewportDragDrop::Get() {
    static ViewportDragDrop instance;
    return instance;
}

void ViewportDragDrop::HandleDragDropTarget(ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]) {
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ASSET_PATH")) {
            const char* assetPathCStr = (const char*)payload->Data;
            if (assetPathCStr && strlen(assetPathCStr) > 0) {
                std::string assetPath = assetPathCStr;
                std::filesystem::path p(assetPath);
                std::string fileName = p.filename().string();

                ImVec2 mousePos = ImGui::GetMousePos();
                RaycastHit hit;
                Vec3f spawnPos(0.0f, 0.5f, 0.0f);
                if (ViewportPicker::Get().RaycastSurface(mousePos, cursorPos, viewportAvail, view, proj, hit)) {
                    spawnPos = hit.point;
                }

                SceneNode newNode;
                newNode.id = SceneGraph::Get().GenerateNodeId();
                newNode.name = fileName + "_" + std::to_string(rand() % 1000);
                newNode.type = SceneNodeType::Actor;
                newNode.meshPath = assetPath;
                newNode.location[0] = spawnPos.x;
                newNode.location[1] = spawnPos.y;
                newNode.location[2] = spawnPos.z;

                SceneGraph::Get().AddNode(newNode);
                ViewportSelection::Get().SelectSingle(newNode.name);
                Logger::Get().Info("[Viewport] Drag-and-dropped asset '" + fileName + "' into 3D scene at surface hit!");
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void ViewportDragDrop::RenderGhostPreview(ImDrawList* /*drawList*/, ImVec2 /*cursorPos*/, ImVec2 /*viewportAvail*/, const float /*view*/[16], const float /*proj*/[16]) {
}

} // namespace EngineEditor::Panels
