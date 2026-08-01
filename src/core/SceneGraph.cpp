#include "SceneGraph.h"
#include "third_party/IconsFontAwesome6.h"
#include <algorithm>

namespace EngineEditor {

SceneGraph& SceneGraph::Get() {
    static SceneGraph instance;
    return instance;
}

SceneGraph::SceneGraph() {
    // Start with clean, un-seeded scene graph port
}

void SceneGraph::AddNode(const SceneNode& node) {
    m_RootNodes.push_back(node);
}

bool SceneGraph::RemoveNode(const std::string& name) {
    auto it = std::remove_if(m_RootNodes.begin(), m_RootNodes.end(), [&](const SceneNode& n) {
        return n.name == name;
    });
    if (it != m_RootNodes.end()) {
        m_RootNodes.erase(it, m_RootNodes.end());
        return true;
    }
    return false;
}

void SceneGraph::Clear() {
    m_RootNodes.clear();
}

void SceneGraph::SetRootNodes(const std::vector<SceneNode>& nodes) {
    m_RootNodes = nodes;
}

const SceneNode* SceneGraph::FindNode(const std::string& name, const std::vector<SceneNode>* nodes) const {
    if (nodes == nullptr) {
        nodes = &m_RootNodes;
    }
    for (const auto& node : *nodes) {
        if (node.name == name) return &node;
        const SceneNode* found = FindNode(name, &node.children);
        if (found) return found;
    }
    return nullptr;
}

const char* SceneGraph::GetTypeName(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return "Folder";
        case SceneNodeType::Actor:         return "Actor";
        case SceneNodeType::Light:         return "Light";
        case SceneNodeType::Camera:        return "Camera";
        case SceneNodeType::Audio:         return "Audio";
        case SceneNodeType::SkyAtmosphere: return "SkyAtmosphere";
        case SceneNodeType::Component:     return "Component";
        default:                           return "Object";
    }
}

ImVec4 SceneGraph::GetTypeColor(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // Light Gray
        case SceneNodeType::Actor:         return ImVec4(0.30f, 0.75f, 0.95f, 1.00f); // Cyan
        case SceneNodeType::Light:         return ImVec4(0.95f, 0.80f, 0.25f, 1.00f); // Amber Yellow
        case SceneNodeType::Camera:        return ImVec4(0.40f, 0.85f, 0.50f, 1.00f); // Green
        case SceneNodeType::Audio:         return ImVec4(0.90f, 0.45f, 0.25f, 1.00f); // Orange
        case SceneNodeType::SkyAtmosphere: return ImVec4(0.70f, 0.45f, 0.95f, 1.00f); // Purple
        case SceneNodeType::Component:     return ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // Muted
        default:                           return ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    }
}

const char* SceneGraph::GetTypeIconTag(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return ICON_FA_FOLDER;
        case SceneNodeType::Actor:         return ICON_FA_CUBE;
        case SceneNodeType::Light:         return ICON_FA_LIGHTBULB;
        case SceneNodeType::Camera:        return ICON_FA_CAMERA;
        case SceneNodeType::Audio:         return ICON_FA_VOLUME_HIGH;
        case SceneNodeType::SkyAtmosphere: return ICON_FA_SUN;
        case SceneNodeType::Component:     return ICON_FA_SLIDERS;
        default:                           return ICON_FA_CUBE;
    }
}

} // namespace EngineEditor
