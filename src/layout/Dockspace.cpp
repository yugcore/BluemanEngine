#include "Dockspace.h"
#include <imgui_internal.h>

namespace EngineEditor::Layout {

DockspaceBounds CalculateDockspaceBounds(float topOffset, float bottomOffset) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    DockspaceBounds bounds;
    bounds.pos = ImVec2(viewport->Pos.x, viewport->Pos.y + topOffset);
    bounds.size = ImVec2(viewport->Size.x, viewport->Size.y - topOffset - bottomOffset);
    return bounds;
}

ImGuiID RenderDockspaceHost(const DockspaceBounds& bounds, ImGuiDockNodeFlags flags) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(bounds.pos);
    ImGui::SetNextWindowSize(bounds.size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags hostFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
                                 ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("##MainEngineDockspaceHost", nullptr, hostFlags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspaceId = ImGui::GetID("EngineDockspace");
    ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), flags);

    ImGui::End();

    return dockspaceId;
}

} // namespace EngineEditor::Layout
