#include "CodeSymbolsPanel.h"
#include "core/EditorState.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <imgui.h>
#include <string>

namespace EngineEditor {

void RenderCodeSymbolsPanel(bool* pOpen) {
    if (!ImGui::Begin("API Library", pOpen, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImGui::End();
}

} // namespace EngineEditor
