#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <string>
#include <vector>
#include <imgui.h>

namespace EngineEditor {

enum class SceneNodeType {
    Folder,
    Actor,
    Light,
    Camera,
    Audio,
    SkyAtmosphere,
    Component
};

struct SceneNode {
    std::string name;
    SceneNodeType type;
    std::string world = "DefaultWorld";
    std::string panel = "MainPanel";
    std::vector<SceneNode> children;
};

class SceneGraph {
public:
    static SceneGraph& Get();

    SceneGraph();

    const std::vector<SceneNode>& GetRootNodes() const { return m_RootNodes; }
    
    // Find node by name
    const SceneNode* FindNode(const std::string& name, const std::vector<SceneNode>* nodes = nullptr) const;

    static const char* GetTypeName(SceneNodeType type);
    static ImVec4 GetTypeColor(SceneNodeType type);
    static const char* GetTypeIconTag(SceneNodeType type);

    // Runtime Ports for Engine Integration
    void AddNode(const SceneNode& node);
    bool RemoveNode(const std::string& name);
    void Clear();
    void SetRootNodes(const std::vector<SceneNode>& nodes);

private:
    std::vector<SceneNode> m_RootNodes;
};

} // namespace EngineEditor

#endif // SCENE_GRAPH_H
