#include "RunViewportPanel.h"
#include "render/ViewportRenderer.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>

namespace EngineEditor {

void RenderRunViewportPanel(bool* pOpen) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    if (!ImGui::Begin("Running Game", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        ImGui::PopStyleVar();
        return;
    }

    ImVec2 viewportAvail = ImGui::GetContentRegionAvail();
    uint32_t width = (uint32_t)viewportAvail.x;
    uint32_t height = (uint32_t)viewportAvail.y;

    if (width > 0 && height > 0) {
        ViewportRenderer::Get().Resize(width, height);
    }

    float deltaTime = ImGui::GetIO().DeltaTime;
    ViewportRenderer::Get().RenderScene(deltaTime);

    uint64_t textureID = ViewportRenderer::Get().GetTextureID();
    if (textureID != 0) {
        ImGui::Image((ImTextureID)textureID, viewportAvail, ImVec2(0, 1), ImVec2(1, 0));
    }

    ImGui::End();
    ImGui::PopStyleVar();
}

} // namespace EngineEditor
