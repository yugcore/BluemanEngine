#include "ViewportSelection.h"
#include "ViewportPicker.h"
#include "core/EditorState.h"
#include "core/SceneGraph.h"
#include "core/CommandStack.h"
#include <algorithm>

namespace EngineEditor::Panels {

ViewportSelection& ViewportSelection::Get() {
    static ViewportSelection instance;
    return instance;
}

void ViewportSelection::SelectSingle(const std::string& nodeName, bool add, bool toggle) {
    std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
    std::vector<std::string> newSel = oldSel;

    if (toggle) {
        auto it = std::find(newSel.begin(), newSel.end(), nodeName);
        if (it != newSel.end()) {
            newSel.erase(it);
        } else if (!nodeName.empty()) {
            newSel.push_back(nodeName);
        }
    } else if (add) {
        if (!nodeName.empty() && std::find(newSel.begin(), newSel.end(), nodeName) == newSel.end()) {
            newSel.push_back(nodeName);
        }
    } else {
        newSel.clear();
        if (!nodeName.empty()) {
            newSel.push_back(nodeName);
        }
    }

    if (oldSel != newSel) {
        auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
        CommandStack::Get().PushAndExecute(cmd);
    }
}

void ViewportSelection::SelectAll() {
    std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
    std::vector<std::string> newSel;

    auto gatherNodes = [&](auto& self, const std::vector<SceneNode>& nodes) -> void {
        for (const auto& n : nodes) {
            newSel.push_back(n.name);
            if (!n.children.empty()) self(self, n.children);
        }
    };
    gatherNodes(gatherNodes, SceneGraph::Get().GetRootNodes());

    if (oldSel != newSel) {
        auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
        CommandStack::Get().PushAndExecute(cmd);
    }
}

void ViewportSelection::InvertSelection() {
    std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
    std::vector<std::string> newSel;

    auto gatherInverted = [&](auto& self, const std::vector<SceneNode>& nodes) -> void {
        for (const auto& n : nodes) {
            if (std::find(oldSel.begin(), oldSel.end(), n.name) == oldSel.end()) {
                newSel.push_back(n.name);
            }
            if (!n.children.empty()) self(self, n.children);
        }
    };
    gatherInverted(gatherInverted, SceneGraph::Get().GetRootNodes());

    auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
    CommandStack::Get().PushAndExecute(cmd);
}

void ViewportSelection::ClearSelection() {
    std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
    if (!oldSel.empty()) {
        std::vector<std::string> newSel;
        auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
        CommandStack::Get().PushAndExecute(cmd);
    }
}

void ViewportSelection::SelectParent() {
    std::string primary = EditorState::Get().selectedNodeName;
    if (primary.empty()) return;

    // Search for parent of node
    auto findParent = [&](auto& self, const std::vector<SceneNode>& nodes, const std::string& target) -> const SceneNode* {
        for (const auto& parent : nodes) {
            for (const auto& child : parent.children) {
                if (child.name == target) return &parent;
            }
            const SceneNode* p = self(self, parent.children, target);
            if (p) return p;
        }
        return nullptr;
    };

    const SceneNode* parentNode = findParent(findParent, SceneGraph::Get().GetRootNodes(), primary);
    if (parentNode) {
        SelectSingle(parentNode->name);
    }
}

void ViewportSelection::SelectChildren() {
    std::string primary = EditorState::Get().selectedNodeName;
    if (primary.empty()) return;

    const SceneNode* node = SceneGraph::Get().FindNode(primary);
    if (node && !node->children.empty()) {
        std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
        std::vector<std::string> newSel;
        for (const auto& child : node->children) {
            newSel.push_back(child.name);
        }
        auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
        CommandStack::Get().PushAndExecute(cmd);
    }
}

void ViewportSelection::StartMarquee(ImVec2 startPos) {
    m_IsMarqueeActive = true;
    m_MarqueeStart = startPos;
    m_MarqueeCurrent = startPos;
}

void ViewportSelection::UpdateMarquee(ImVec2 currentPos) {
    if (m_IsMarqueeActive) {
        m_MarqueeCurrent = currentPos;
    }
}

void ViewportSelection::EndMarquee(ImVec2 cursorPos, ImVec2 viewportAvail, const float view[16], const float proj[16], bool shiftHeld, bool ctrlHeld) {
    if (!m_IsMarqueeActive) return;
    m_IsMarqueeActive = false;

    float dx = std::abs(m_MarqueeCurrent.x - m_MarqueeStart.x);
    float dy = std::abs(m_MarqueeCurrent.y - m_MarqueeStart.y);

    if (dx < 4.0f && dy < 4.0f) {
        // Less than 4px drag is a simple click selection
        RaycastHit hit;
        if (ViewportPicker::Get().PickNode(m_MarqueeStart, cursorPos, viewportAvail, view, proj, hit)) {
            SelectSingle(hit.nodeName, shiftHeld, ctrlHeld);
        } else if (!shiftHeld && !ctrlHeld) {
            ClearSelection();
        }
        return;
    }

    ImVec2 minRect(std::min(m_MarqueeStart.x, m_MarqueeCurrent.x), std::min(m_MarqueeStart.y, m_MarqueeCurrent.y));
    ImVec2 maxRect(std::max(m_MarqueeStart.x, m_MarqueeCurrent.x), std::max(m_MarqueeStart.y, m_MarqueeCurrent.y));

    std::vector<std::string> oldSel = EditorState::Get().selectedNodeNames;
    std::vector<std::string> newSel = (shiftHeld || ctrlHeld) ? oldSel : std::vector<std::string>();

    auto checkNodeIntersection = [&](auto& self, const SceneNode& node) -> void {
        AABB box;
        float halfX = std::max(0.5f, std::abs(node.scale[0]) * 0.5f);
        float halfY = std::max(0.5f, std::abs(node.scale[1]) * 0.5f);
        float halfZ = std::max(0.5f, std::abs(node.scale[2]) * 0.5f);
        box.minBounds = Vec3f(node.location[0] - halfX, node.location[1] - halfY, node.location[2] - halfZ);
        box.maxBounds = Vec3f(node.location[0] + halfX, node.location[1] + halfY, node.location[2] + halfZ);

        if (ViewportMath::ScreenRectIntersectsAABB(minRect, maxRect, box, view, proj, cursorPos, viewportAvail)) {
            if (ctrlHeld) {
                auto it = std::find(newSel.begin(), newSel.end(), node.name);
                if (it != newSel.end()) newSel.erase(it);
                else newSel.push_back(node.name);
            } else {
                if (std::find(newSel.begin(), newSel.end(), node.name) == newSel.end()) {
                    newSel.push_back(node.name);
                }
            }
        }

        for (const auto& child : node.children) {
            self(self, child);
        }
    };

    for (const auto& root : SceneGraph::Get().GetRootNodes()) {
        checkNodeIntersection(checkNodeIntersection, root);
    }

    if (oldSel != newSel) {
        auto cmd = std::make_shared<SelectionChangeCommand>(oldSel, newSel);
        CommandStack::Get().PushAndExecute(cmd);
    }
}

void ViewportSelection::RenderMarquee(ImDrawList* drawList) {
    if (!m_IsMarqueeActive || !drawList) return;

    ImVec2 minRect(std::min(m_MarqueeStart.x, m_MarqueeCurrent.x), std::min(m_MarqueeStart.y, m_MarqueeCurrent.y));
    ImVec2 maxRect(std::max(m_MarqueeStart.x, m_MarqueeCurrent.x), std::max(m_MarqueeStart.y, m_MarqueeCurrent.y));

    // Translucent filled rectangle
    drawList->AddRectFilled(minRect, maxRect, IM_COL32(70, 160, 245, 45));
    // Crisp border outline
    drawList->AddRect(minRect, maxRect, IM_COL32(100, 190, 255, 230), 0.0f, 0, 1.5f);
}

} // namespace EngineEditor::Panels
