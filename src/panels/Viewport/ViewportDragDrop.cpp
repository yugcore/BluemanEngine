#include "ViewportDragDrop.h"
#include "ViewportPicker.h"
#include "ViewportSelection.h"
#include "ViewportMath.h"
#include "core/EditorState.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include "engine/assets/BackgroundAssetCooker.h"
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
                std::string ext = p.extension().string();

                ImVec2 mousePos = ImGui::GetMousePos();
                RaycastHit hit;
                Vec3f spawnPos(0.0f, 0.5f, 0.0f);
                if (ViewportPicker::Get().RaycastSurface(mousePos, cursorPos, viewportAvail, view, proj, hit)) {
                    spawnPos = hit.point;
                }

                // Handle Material (.zmat / .mat) Drag and Drop onto target entity
                if (ext == ".zmat" || ext == ".mat" || ext == ".material") {
                    if (ViewportPicker::Get().PickNode(mousePos, cursorPos, viewportAvail, view, proj, hit)) {
                        SceneNode* hitNode = SceneGraph::Get().FindNodeMutable(hit.nodeName);
                        if (hitNode) {
                            hitNode->materialPath = assetPath;
                            SceneGraph::Get().SyncNodeComponents(*hitNode);
                            Logger::Get().Info("[Viewport] Applied material '" + fileName + "' to node '" + hit.nodeName + "'");
                        }
                    }
                } else if (ext == ".zmesh" || ext == ".zasset") {
                    // Pre-cooked mesh format: Create SceneNode immediately
                    SceneNode newNode;
                    newNode.id = SceneGraph::Get().GenerateNodeId();
                    newNode.name = p.stem().string() + "_" + std::to_string(rand() % 1000);
                    newNode.type = SceneNodeType::Actor;
                    newNode.meshPath = assetPath;

                    // Auto-assign matching .zmat material if present in same directory
                    std::filesystem::path matPath = p;
                    matPath.replace_extension(".zmat");
                    if (std::filesystem::exists(matPath)) {
                        newNode.materialPath = matPath.string();
                    }

                    newNode.location[0] = spawnPos.x;
                    newNode.location[1] = spawnPos.y;
                    newNode.location[2] = spawnPos.z;

                    SceneGraph::Get().AddNode(newNode);
                    ViewportSelection::Get().SelectSingle(newNode.name);
                    Logger::Get().Info("[Viewport] Drag-and-dropped pre-cooked mesh asset '" + fileName + "' into 3D scene at surface hit!");
                } else {
                    // Raw asset formats (.gltf, .glb, .fbx, .obj, .vox, etc.) requiring cooking:
                    // Queue for background cooking and defer SceneNode instantiation to BackgroundAssetCooker::Update()
                    BackgroundAssetCooker::Get().QueueFileForCooking(assetPath, spawnPos.x, spawnPos.y, spawnPos.z);
                    Logger::Get().Info("[Viewport] Queued raw asset '" + fileName + "' for cooking before scene instantiation at surface hit (" +
                                       std::to_string(spawnPos.x) + ", " + std::to_string(spawnPos.y) + ", " + std::to_string(spawnPos.z) + ")");
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

void ViewportDragDrop::RenderGhostPreview(ImDrawList* drawList, ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16]) {
    if (!drawList || viewportAvail.x <= 0.0f || viewportAvail.y <= 0.0f) return;

    const ImGuiPayload* payload = ImGui::GetDragDropPayload();
    if (!payload || !payload->IsDataType("CONTENT_BROWSER_ASSET_PATH")) return;

    ImVec2 mousePos = ImGui::GetMousePos();
    RaycastHit hit;
    if (ViewportPicker::Get().RaycastSurface(mousePos, cursorPos, viewportAvail, view, proj, hit)) {
        ImVec2 screenPos;
        if (ViewportMath::WorldToScreen(hit.point, view, proj, cursorPos, viewportAvail, screenPos)) {
            drawList->AddCircleFilled(screenPos, 8.0f, IM_COL32(70, 160, 245, 120));
            drawList->AddCircle(screenPos, 14.0f, IM_COL32(100, 190, 255, 220), 0, 2.0f);

            const char* assetPathCStr = (const char*)payload->Data;
            if (assetPathCStr && strlen(assetPathCStr) > 0) {
                std::filesystem::path p(assetPathCStr);
                std::string label = "+ " + p.filename().string();
                drawList->AddText(ImVec2(screenPos.x + 18.0f, screenPos.y - 8.0f), IM_COL32(255, 255, 255, 230), label.c_str());
            }
        }
    }
}

} // namespace EngineEditor::Panels
