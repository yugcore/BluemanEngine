#include "AssetRegistry.h"
#include "third_party/IconsFontAwesome6.h"
#include <algorithm>

namespace EngineEditor {

AssetRegistry& AssetRegistry::Get() {
    static AssetRegistry instance;
    return instance;
}

AssetRegistry::AssetRegistry() {
    m_RootFolder.name = "ZeGFX Workspace";
    m_RootFolder.path = "ZeGFX Workspace";
}

void AssetRegistry::RegisterAsset(const AssetItem& item, const std::string& folderPath) {
    if (folderPath.empty() || folderPath == m_RootFolder.path) {
        m_RootFolder.items.push_back(item);
        return;
    }
    AssetFolder* folder = const_cast<AssetFolder*>(FindFolder(folderPath));
    if (folder) {
        folder->items.push_back(item);
    } else {
        m_RootFolder.items.push_back(item);
    }
}

void AssetRegistry::RegisterFolder(const AssetFolder& folder) {
    m_RootFolder.subfolders.push_back(folder);
}

void AssetRegistry::Clear() {
    m_RootFolder.items.clear();
    m_RootFolder.subfolders.clear();
}

void AssetRegistry::SetRootFolder(const AssetFolder& folder) {
    m_RootFolder = folder;
}

const AssetFolder* AssetRegistry::FindFolder(const std::string& path, const AssetFolder* current) const {
    if (current == nullptr) {
        current = &m_RootFolder;
    }
    if (current->path == path) {
        return current;
    }
    for (const auto& sub : current->subfolders) {
        const AssetFolder* found = FindFolder(path, &sub);
        if (found) return found;
    }
    return nullptr;
}

const char* AssetRegistry::GetTypeName(AssetItemType type) {
    switch (type) {
        case AssetItemType::Material:    return "Material";
        case AssetItemType::Mesh:        return "StaticMesh";
        case AssetItemType::Texture:     return "Texture2D";
        case AssetItemType::Script:      return "ZelynScript";
        case AssetItemType::Animation:   return "AnimSequence";
        case AssetItemType::Audio:       return "SoundWave";
        case AssetItemType::Level:       return "Map";
        case AssetItemType::VFX:         return "NiagaraVFX";
        case AssetItemType::Physics:     return "PhysicsAsset";
        case AssetItemType::UI:          return "WidgetBlueprint";
        case AssetItemType::Folder:      return "Folder";
        default:                         return "Asset";
    }
}

ImVec4 AssetRegistry::GetTypeColor(AssetItemType type) {
    switch (type) {
        case AssetItemType::Material:    return ImVec4(0.22f, 0.63f, 0.41f, 1.00f); // Green (#38A169)
        case AssetItemType::Mesh:        return ImVec4(0.19f, 0.51f, 0.81f, 1.00f); // Blue (#3182CE)
        case AssetItemType::Texture:     return ImVec4(0.84f, 0.62f, 0.18f, 1.00f); // Amber/Gold (#D69E2E)
        case AssetItemType::Script:      return ImVec4(0.50f, 0.35f, 0.84f, 1.00f); // Purple (#805AD5)
        case AssetItemType::Animation:   return ImVec4(0.84f, 0.25f, 0.55f, 1.00f); // Pink (#D53F8C)
        case AssetItemType::Audio:       return ImVec4(0.19f, 0.59f, 0.58f, 1.00f); // Teal (#319795)
        case AssetItemType::Level:       return ImVec4(0.90f, 0.24f, 0.24f, 1.00f); // Red (#E53E3E)
        case AssetItemType::VFX:         return ImVec4(0.87f, 0.42f, 0.13f, 1.00f); // Orange (#DD6B20)
        case AssetItemType::Physics:     return ImVec4(0.18f, 0.52f, 0.35f, 1.00f); // Emerald (#2F855A)
        case AssetItemType::UI:          return ImVec4(0.30f, 0.32f, 0.75f, 1.00f); // Indigo (#4C51BF)
        case AssetItemType::Folder:      return ImVec4(0.44f, 0.50f, 0.59f, 1.00f); // Warm Gray (#718096)
        default:                         return ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    }
}

} // namespace EngineEditor
