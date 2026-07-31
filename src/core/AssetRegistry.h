#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <string>
#include <vector>
#include <imgui.h>

namespace EngineEditor {

enum class AssetItemType {
    Mesh,
    Material,
    Texture,
    Blueprint,
    AI,
    LevelScript,
    Unknown
};

struct AssetItem {
    std::string name;
    AssetItemType type;
    std::string path;
};

struct AssetFolder {
    std::string name;
    std::string path;
    std::vector<AssetFolder> subfolders;
    std::vector<AssetItem> items;
};

class AssetRegistry {
public:
    static AssetRegistry& Get();

    AssetRegistry();

    const AssetFolder& GetRootFolder() const { return m_RootFolder; }
    
    // Find folder by path (e.g. "ZeGFX Workspace/Blueman Cooked Assets/Meshes")
    const AssetFolder* FindFolder(const std::string& path, const AssetFolder* current = nullptr) const;

    // Helper functions for asset metadata
    static const char* GetTypeName(AssetItemType type);
    static ImVec4 GetTypeColor(AssetItemType type);

private:
    void SeedDummyData();

    AssetFolder m_RootFolder;
};

} // namespace EngineEditor

#endif // ASSET_REGISTRY_H
