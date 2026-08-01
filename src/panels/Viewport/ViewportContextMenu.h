#ifndef VIEWPORT_CONTEXT_MENU_H
#define VIEWPORT_CONTEXT_MENU_H

#include <imgui.h>
#include <string>

namespace EngineEditor::Panels {

class ViewportContextMenu {
public:
    static ViewportContextMenu& Get();

    void OpenMenu(const std::string& targetNodeName);
    void Render();

private:
    std::string m_TargetNode;
    bool m_ShouldOpen = false;
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_CONTEXT_MENU_H
