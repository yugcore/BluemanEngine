#ifndef ASSET_REGISTRY_H
#define ASSET_REGISTRY_H

#include <string>
#include <vector>
#include <mutex>

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
    bool isDependencyOnly = false;
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

    AssetFolder GetRootFolderCopy() const;
    const AssetFolder& GetRootFolder() const { return m_RootFolder; }
    
    // Find folder by path (e.g. "Z:\Blueman Cooked Assets")
    const AssetFolder* FindFolder(const std::string& path, const AssetFolder* current = nullptr) const;

    // Helper functions for asset metadata
    static const char* GetTypeName(AssetItemType type);
    static AssetItemType DetectItemType(const std::string& extension);
    static bool IsDependencyOnlyExtension(const std::string& extension);

    // Dynamic scanning of cooked asset directory
    void ScanProjectFolder(const std::string& folderPath = "Z:\\Blueman Cooked Assets");

    // Runtime Ports for Engine Integration
    void RegisterAsset(const AssetItem& item, const std::string& folderPath = "");
    void RegisterFolder(const AssetFolder& folder);
    void Clear();
    void SetRootFolder(const AssetFolder& folder);

private:
    mutable std::mutex m_Mutex;
    AssetFolder m_RootFolder;
};

} // namespace EngineEditor

#endif // ASSET_REGISTRY_H
