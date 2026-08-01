#include "SceneGraph.h"
#include "third_party/IconsFontAwesome6.h"
#include "core/Logger.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace EngineEditor {

SceneGraph& SceneGraph::Get() {
    static SceneGraph instance;
    return instance;
}

uint64_t SceneGraph::GenerateNodeId() {
    return m_NextNodeId++;
}

SceneGraph::SceneGraph() {
    SceneNode sunNode;
    sunNode.id = GenerateNodeId();
    sunNode.name = "DirectionalSunLight";
    sunNode.type = SceneNodeType::Light;
    m_RootNodes.push_back(sunNode);

    SceneNode skyNode;
    skyNode.id = GenerateNodeId();
    skyNode.name = "AtmosphericBlueSky";
    skyNode.type = SceneNodeType::SkyAtmosphere;
    m_RootNodes.push_back(skyNode);

    SceneNode gridNode;
    gridNode.id = GenerateNodeId();
    gridNode.name = "FloorGrid_Ground";
    gridNode.type = SceneNodeType::Actor;
    m_RootNodes.push_back(gridNode);

    // Forest Walk Environment Nodes (500m - 1km Playable Area)
    SceneNode terrainNode;
    terrainNode.id = GenerateNodeId();
    terrainNode.name = "Forest_Terrain_1KM";
    terrainNode.type = SceneNodeType::Terrain;
    m_RootNodes.push_back(terrainNode);

    SceneNode foliageNode;
    foliageNode.id = GenerateNodeId();
    foliageNode.name = "Forest_Canopy_Trees_Cluster";
    foliageNode.type = SceneNodeType::FoliageCluster;
    m_RootNodes.push_back(foliageNode);

    SceneNode pathNode;
    pathNode.id = GenerateNodeId();
    pathNode.name = "Forest_Trail_Path_Waypoint_01";
    pathNode.type = SceneNodeType::PathPoint;
    m_RootNodes.push_back(pathNode);
}

void SceneGraph::AddNode(const SceneNode& node) {
    SceneNode n = node;
    if (n.id == 0) {
        n.id = GenerateNodeId();
    }
    m_RootNodes.push_back(n);
}

SceneNode* SceneGraph::DuplicateNode(const std::string& name) {
    const SceneNode* target = FindNode(name);
    if (!target) return nullptr;

    SceneNode dup = *target;
    dup.id = GenerateNodeId();
    dup.name = target->name + "_Copy";
    dup.location[0] += 1.0f;
    dup.location[2] += 1.0f;

    // Helper to fix child IDs recursively
    auto fixChildIds = [this](auto& self, SceneNode& n) -> void {
        for (auto& child : n.children) {
            child.id = this->GenerateNodeId();
            self(self, child);
        }
    };
    fixChildIds(fixChildIds, dup);

    m_RootNodes.push_back(dup);
    return &m_RootNodes.back();
}

SceneNode* SceneGraph::PasteClipboard() {
    if (!m_HasClipboard) return nullptr;
    SceneNode pasted = m_ClipboardNode;
    pasted.id = GenerateNodeId();
    pasted.name = m_ClipboardNode.name + "_Pasted";
    pasted.location[0] += 0.5f;
    pasted.location[2] += 0.5f;

    m_RootNodes.push_back(pasted);
    return &m_RootNodes.back();
}

bool SceneGraph::SaveToFile(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    auto serializeNode = [&out](auto& self, const SceneNode& node, int indent) -> void {
        std::string ind(indent * 2, ' ');
        out << ind << "NODE " << node.id << " " << static_cast<int>(node.type) << " \"" << node.name << "\"\n";
        out << ind << "POS " << node.location[0] << " " << node.location[1] << " " << node.location[2] << "\n";
        out << ind << "ROT " << node.rotation[0] << " " << node.rotation[1] << " " << node.rotation[2] << "\n";
        out << ind << "SCL " << node.scale[0] << " " << node.scale[1] << " " << node.scale[2] << "\n";
        out << ind << "MESH \"" << node.meshPath << "\"\n";
        out << ind << "CHILDREN " << node.children.size() << "\n";
        for (const auto& child : node.children) {
            self(self, child, indent + 1);
        }
    };

    out << "SCENE_GRAPH " << m_RootNodes.size() << "\n";
    for (const auto& node : m_RootNodes) {
        serializeNode(serializeNode, node, 1);
    }

    return true;
}

bool SceneGraph::LoadFromFile(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return false;

    std::string tag;
    size_t rootCount = 0;
    if (!(in >> tag >> rootCount) || tag != "SCENE_GRAPH") return false;

    Clear();

    auto parseNode = [&in](auto& self) -> SceneNode {
        SceneNode n;
        std::string lineTag;
        int typeInt = 0;
        in >> lineTag >> n.id >> typeInt;
        n.type = static_cast<SceneNodeType>(typeInt);

        // Read name in quotes
        in >> std::ws;
        char quote = in.get();
        if (quote == '"') {
            std::getline(in, n.name, '"');
        } else {
            in.unget();
            in >> n.name;
        }

        in >> lineTag >> n.location[0] >> n.location[1] >> n.location[2];
        in >> lineTag >> n.rotation[0] >> n.rotation[1] >> n.rotation[2];
        in >> lineTag >> n.scale[0] >> n.scale[1] >> n.scale[2];

        in >> lineTag >> std::ws;
        quote = in.get();
        if (quote == '"') {
            std::getline(in, n.meshPath, '"');
        } else {
            in.unget();
            in >> n.meshPath;
        }

        size_t childCount = 0;
        in >> lineTag >> childCount;
        for (size_t i = 0; i < childCount; ++i) {
            n.children.push_back(self(self));
        }
        return n;
    };

    for (size_t i = 0; i < rootCount; ++i) {
        m_RootNodes.push_back(parseNode(parseNode));
    }

    return true;
}

bool SceneGraph::RemoveNodeRecursive(std::vector<SceneNode>& nodes, const std::string& name) {
    auto it = std::remove_if(nodes.begin(), nodes.end(), [&](const SceneNode& n) {
        return n.name == name;
    });
    if (it != nodes.end()) {
        nodes.erase(it, nodes.end());
        return true;
    }
    // Search children recursively
    for (auto& node : nodes) {
        if (RemoveNodeRecursive(node.children, name)) return true;
    }
    return false;
}

bool SceneGraph::RemoveNode(const std::string& name) {
    return RemoveNodeRecursive(m_RootNodes, name);
}

bool SceneGraph::RemoveNodeByIdRecursive(std::vector<SceneNode>& nodes, uint64_t id) {
    auto it = std::remove_if(nodes.begin(), nodes.end(), [&](const SceneNode& n) {
        return n.id == id;
    });
    if (it != nodes.end()) {
        nodes.erase(it, nodes.end());
        return true;
    }
    for (auto& node : nodes) {
        if (RemoveNodeByIdRecursive(node.children, id)) return true;
    }
    return false;
}

bool SceneGraph::RemoveNodeById(uint64_t id) {
    return RemoveNodeByIdRecursive(m_RootNodes, id);
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

SceneNode* SceneGraph::FindNodeMutable(const std::string& name, std::vector<SceneNode>* nodes) {
    if (nodes == nullptr) {
        nodes = &m_RootNodes;
    }
    for (auto& node : *nodes) {
        if (node.name == name) return &node;
        SceneNode* found = FindNodeMutable(name, &node.children);
        if (found) return found;
    }
    return nullptr;
}

const SceneNode* SceneGraph::FindNodeById(uint64_t id, const std::vector<SceneNode>* nodes) const {
    if (nodes == nullptr) {
        nodes = &m_RootNodes;
    }
    for (const auto& node : *nodes) {
        if (node.id == id) return &node;
        const SceneNode* found = FindNodeById(id, &node.children);
        if (found) return found;
    }
    return nullptr;
}

SceneNode* SceneGraph::FindNodeByIdMutable(uint64_t id, std::vector<SceneNode>* nodes) {
    if (nodes == nullptr) {
        nodes = &m_RootNodes;
    }
    for (auto& node : *nodes) {
        if (node.id == id) return &node;
        SceneNode* found = FindNodeByIdMutable(id, &node.children);
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
        case SceneNodeType::Terrain:       return "Terrain";
        case SceneNodeType::FoliageCluster:return "FoliageCluster";
        case SceneNodeType::PathPoint:     return "PathPoint";
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
        case SceneNodeType::Terrain:       return ImVec4(0.45f, 0.75f, 0.35f, 1.00f); // Forest Green
        case SceneNodeType::FoliageCluster:return ImVec4(0.35f, 0.85f, 0.45f, 1.00f); // Emerald
        case SceneNodeType::PathPoint:     return ImVec4(0.85f, 0.65f, 0.35f, 1.00f); // Soil Brown
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
        case SceneNodeType::Terrain:       return ICON_FA_MOUNTAIN;
        case SceneNodeType::FoliageCluster:return ICON_FA_TREE;
        case SceneNodeType::PathPoint:     return ICON_FA_LOCATION_DOT;
        default:                           return ICON_FA_CUBE;
    }
}

} // namespace EngineEditor
