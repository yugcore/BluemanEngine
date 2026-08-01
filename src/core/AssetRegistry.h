#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <string>
#include <vector>
#include <imgui.h>

namespace EngineEditor {

enum class AssetItemType {
    Material,
    Mesh,
    Texture,
    Script,
    Animation,
    Audio,
    Level,
    VFX,
    Physics,
    UI,
    Folder,
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

    // Runtime Ports for Engine Integration
    void RegisterAsset(const AssetItem& item, const std::string& folderPath = "");
    void RegisterFolder(const AssetFolder& folder);
    void Clear();
    void SetRootFolder(const AssetFolder& folder);

private:
    AssetFolder m_RootFolder;
};

} // namespace EngineEditor

#endif // ASSET_REGISTRY_H
