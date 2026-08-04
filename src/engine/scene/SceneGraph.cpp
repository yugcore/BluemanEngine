#include "engine/scene/SceneGraph.h"
#include "third_party/IconsFontAwesome6.h"
#include "engine/core/Logger.h"
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

size_t SceneGraph::GetTotalNodeCount(const std::vector<SceneNode>* nodes) const {
    const auto& list = nodes ? *nodes : m_RootNodes;
    size_t count = list.size();
    for (const auto& node : list) {
        if (!node.children.empty()) {
            count += GetTotalNodeCount(&node.children);
        }
    }
    return count;
}

void SceneGraph::SyncNodeComponents(SceneNode& node) {
    if (node.id == 0) return;

    TransformComponent* transformComp = ComponentRegistry::Get().GetComponent<TransformComponent>(node.id);
    if (!transformComp) {
        TransformComponent newTransform;
        newTransform.location[0] = node.location[0];
        newTransform.location[1] = node.location[1];
        newTransform.location[2] = node.location[2];
        newTransform.rotation[0] = node.rotation[0];
        newTransform.rotation[1] = node.rotation[1];
        newTransform.rotation[2] = node.rotation[2];
        newTransform.scale[0]    = node.scale[0];
        newTransform.scale[1]    = node.scale[1];
        newTransform.scale[2]    = node.scale[2];
        ComponentRegistry::Get().AddComponent<TransformComponent>(node.id, newTransform);
    } else {
        transformComp->location[0] = node.location[0];
        transformComp->location[1] = node.location[1];
        transformComp->location[2] = node.location[2];
        transformComp->rotation[0] = node.rotation[0];
        transformComp->rotation[1] = node.rotation[1];
        transformComp->rotation[2] = node.rotation[2];
        transformComp->scale[0]    = node.scale[0];
        transformComp->scale[1]    = node.scale[1];
        transformComp->scale[2]    = node.scale[2];
    }

    if (node.type == SceneNodeType::Actor || node.type == SceneNodeType::Terrain ||
        node.type == SceneNodeType::FoliageCluster || node.type == SceneNodeType::PathPoint || !node.meshPath.empty()) {
        MeshComponent* meshComp = ComponentRegistry::Get().GetComponent<MeshComponent>(node.id);
        if (!meshComp) {
            MeshComponent newMesh;
            newMesh.meshPath = node.meshPath;
            ComponentRegistry::Get().AddComponent<MeshComponent>(node.id, newMesh);
        } else if (!node.meshPath.empty()) {
            meshComp->meshPath = node.meshPath;
        }
    }

    if (node.type == SceneNodeType::Actor || node.type == SceneNodeType::Terrain || !node.materialPath.empty()) {
        MaterialComponent* matComp = ComponentRegistry::Get().GetComponent<MaterialComponent>(node.id);
        if (!matComp) {
            MaterialComponent newMat;
            newMat.materialPath = node.materialPath;
            ComponentRegistry::Get().AddComponent<MaterialComponent>(node.id, newMat);
        } else if (!node.materialPath.empty()) {
            matComp->materialPath = node.materialPath;
        }
    }

    if (node.type == SceneNodeType::Light) {
        LightComponent* lightComp = ComponentRegistry::Get().GetComponent<LightComponent>(node.id);
        if (!lightComp) {
            LightComponent newLight;
            newLight.lightType = 0; // Default Directional Sun Light
            newLight.intensity = 100000.0f;
            newLight.color[0] = 1.0f; newLight.color[1] = 0.95f; newLight.color[2] = 0.85f;
            ComponentRegistry::Get().AddComponent<LightComponent>(node.id, newLight);
        }
    }

    for (auto& child : node.children) {
        SyncNodeComponents(child);
    }
}

SceneGraph::SceneGraph() {
    SceneNode sunNode;
    sunNode.id = GenerateNodeId();
    sunNode.name = "DirectionalSunLight";
    sunNode.type = SceneNodeType::Light;
    sunNode.location[0] = 0.0f; sunNode.location[1] = 10.0f; sunNode.location[2] = 0.0f;
    sunNode.rotation[0] = 53.0f; sunNode.rotation[1] = -59.0f; sunNode.rotation[2] = 0.0f;
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
    gridNode.location[0] = 0.0f; gridNode.location[1] = 0.0f; gridNode.location[2] = 0.0f;
    gridNode.scale[0] = 1.0f; gridNode.scale[1] = 1.0f; gridNode.scale[2] = 1.0f;
    gridNode.meshPath = "Engine/DefaultPlane";
    gridNode.materialPath = "DefaultPBRMaterial";
    m_RootNodes.push_back(gridNode);

    SceneNode cubeNode;
    cubeNode.id = GenerateNodeId();
    cubeNode.name = "DefaultCube";
    cubeNode.type = SceneNodeType::Actor;
    cubeNode.location[0] = 0.0f; cubeNode.location[1] = 1.0f; cubeNode.location[2] = 0.0f;
    cubeNode.scale[0] = 1.0f; cubeNode.scale[1] = 1.0f; cubeNode.scale[2] = 1.0f;
    cubeNode.meshPath = "Engine/DefaultCube";
    cubeNode.materialPath = "DefaultPBRMaterial";
    m_RootNodes.push_back(cubeNode);

    // Forest Walk Environment Nodes (500m - 1km Playable Area)
    SceneNode terrainNode;
    terrainNode.id = GenerateNodeId();
    terrainNode.name = "Forest_Terrain_1KM";
    terrainNode.type = SceneNodeType::Terrain;
    terrainNode.location[0] = 0.0f; terrainNode.location[1] = -0.01f; terrainNode.location[2] = 0.0f;
    terrainNode.scale[0] = 10.0f; terrainNode.scale[1] = 1.0f; terrainNode.scale[2] = 10.0f;
    terrainNode.meshPath = "Engine/DefaultPlane";
    terrainNode.materialPath = "DefaultPBRMaterial";
    m_RootNodes.push_back(terrainNode);

    SceneNode foliageNode;
    foliageNode.id = GenerateNodeId();
    foliageNode.name = "Forest_Canopy_Trees_Cluster";
    foliageNode.type = SceneNodeType::FoliageCluster;
    foliageNode.treeCount = 100;
    foliageNode.meshPath = "Engine/DefaultCone";
    foliageNode.materialPath = "DefaultPBRMaterial";
    m_RootNodes.push_back(foliageNode);

    SceneNode pathNode;
    pathNode.id = GenerateNodeId();
    pathNode.name = "Forest_Trail_Path_Waypoint_01";
    pathNode.type = SceneNodeType::PathPoint;
    m_RootNodes.push_back(pathNode);

    for (auto& root : m_RootNodes) {
        SyncNodeComponents(root);
    }
}

void SceneGraph::AddNode(const SceneNode& node) {
    SceneNode n = node;
    if (n.id == 0) {
        n.id = GenerateNodeId();
    }
    SyncNodeComponents(n);
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

    SyncNodeComponents(dup);
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

    SyncNodeComponents(pasted);
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
        SceneNode node = parseNode(parseNode);
        SyncNodeComponents(node);
        m_RootNodes.push_back(node);
    }

    return true;
}

bool SceneGraph::RemoveNodeRecursive(std::vector<SceneNode>& nodes, const std::string& name) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->name == name) {
            ComponentRegistry::Get().RemoveAllComponents(it->id);
            nodes.erase(it);
            return true;
        }
        if (RemoveNodeRecursive(it->children, name)) return true;
    }
    return false;
}

bool SceneGraph::RemoveNode(const std::string& name) {
    return RemoveNodeRecursive(m_RootNodes, name);
}

bool SceneGraph::RemoveNodeByIdRecursive(std::vector<SceneNode>& nodes, uint64_t id) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->id == id) {
            ComponentRegistry::Get().RemoveAllComponents(it->id);
            nodes.erase(it);
            return true;
        }
        if (RemoveNodeByIdRecursive(it->children, id)) return true;
    }
    return false;
}

bool SceneGraph::RemoveNodeById(uint64_t id) {
    return RemoveNodeByIdRecursive(m_RootNodes, id);
}

void SceneGraph::Clear() {
    ComponentRegistry::Get().Clear();
    m_RootNodes.clear();
}

void SceneGraph::SetRootNodes(const std::vector<SceneNode>& nodes) {
    ComponentRegistry::Get().Clear();
    m_RootNodes = nodes;
    for (auto& root : m_RootNodes) {
        SyncNodeComponents(root);
    }
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
