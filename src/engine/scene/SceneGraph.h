#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <string>
#include <vector>
#include <cstdint>
#include "core/ComponentRegistry.h"

namespace EngineEditor {

enum class SceneNodeType {
    Folder,
    Actor,
    Light,
    Camera,
    Audio,
    SkyAtmosphere,
    Component,
    Terrain,
    FoliageCluster,
    PathPoint
};

struct SceneNode {
    uint64_t id = 0;       // Unique node ID (monotonic counter)
    std::string name;
    SceneNodeType type;
    std::string world = "DefaultWorld";
    std::string panel = "MainPanel";
    std::string meshPath = "";
    std::string materialPath = "";
    float location[3] = { 0.0f, 0.0f, 0.0f };
    float rotation[3] = { 0.0f, 0.0f, 0.0f };
    float scale[3]    = { 1.0f, 1.0f, 1.0f };
    int treeCount     = 100; // Foliage/tree instance count for FoliageCluster nodes
    std::vector<SceneNode> children;
};

class SceneGraph {
public:
    static SceneGraph& Get();

    SceneGraph();

    const std::vector<SceneNode>& GetRootNodes() const { return m_RootNodes; }
    size_t GetTotalNodeCount(const std::vector<SceneNode>* nodes = nullptr) const;
    
    // Find node by name
    const SceneNode* FindNode(const std::string& name, const std::vector<SceneNode>* nodes = nullptr) const;
    SceneNode* FindNodeMutable(const std::string& name, std::vector<SceneNode>* nodes = nullptr);

    // Find node by unique ID
    const SceneNode* FindNodeById(uint64_t id, const std::vector<SceneNode>* nodes = nullptr) const;
    SceneNode* FindNodeByIdMutable(uint64_t id, std::vector<SceneNode>* nodes = nullptr);

    static const char* GetTypeName(SceneNodeType type);
    static const char* GetTypeIconTag(SceneNodeType type);

    // Runtime Ports for Engine Integration
    void AddNode(const SceneNode& node);
    bool RemoveNode(const std::string& name);
    bool RemoveNodeById(uint64_t id);
    SceneNode* DuplicateNode(const std::string& name);
    void Clear();
    void SetRootNodes(const std::vector<SceneNode>& nodes);

    // Serialization & Persistence
    bool SaveToFile(const std::string& filepath) const;
    bool LoadFromFile(const std::string& filepath);

    // Clipboard
    void SetClipboard(const SceneNode& node) { m_ClipboardNode = node; m_HasClipboard = true; }
    bool HasClipboard() const { return m_HasClipboard; }
    const SceneNode& GetClipboard() const { return m_ClipboardNode; }
    SceneNode* PasteClipboard();

    // ID generation
    uint64_t GenerateNodeId();

    // Component ECS Integration
    void SyncNodeComponents(SceneNode& node);

private:
    std::vector<SceneNode> m_RootNodes;
    uint64_t m_NextNodeId = 1;
    SceneNode m_ClipboardNode;
    bool m_HasClipboard = false;

    // Recursive removal helper
    bool RemoveNodeRecursive(std::vector<SceneNode>& nodes, const std::string& name);
    bool RemoveNodeByIdRecursive(std::vector<SceneNode>& nodes, uint64_t id);
};

} // namespace EngineEditor

#endif // SCENE_GRAPH_H
