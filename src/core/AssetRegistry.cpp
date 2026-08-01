#include "AssetRegistry.h"
#include "third_party/IconsFontAwesome6.h"
#include <algorithm>
#include <filesystem>
#include <cctype>

namespace EngineEditor {

namespace fs = std::filesystem;

AssetRegistry& AssetRegistry::Get() {
    static AssetRegistry instance;
    return instance;
}

AssetRegistry::AssetRegistry() {
    m_RootFolder.name = "Blueman Cooked Assets";
    m_RootFolder.path = "Z:\\Blueman Cooked Assets";
    ScanProjectFolder("Z:\\Blueman Cooked Assets");
}

AssetFolder AssetRegistry::GetRootFolderCopy() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_RootFolder;
}

AssetItemType AssetRegistry::DetectItemType(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    if (ext == ".fbx" || ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".vox" || ext == ".zmesh" || ext == ".zasset") {
        return AssetItemType::Mesh;
    }
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds" || ext == ".bmp" || ext == ".hdr" || ext == ".exr" || ext == ".ztex") {
        return AssetItemType::Texture;
    }
    if (ext == ".zmat" || ext == ".mat" || ext == ".material") {
        return AssetItemType::Material;
    }
    if (ext == ".zelyn" || ext == ".lua" || ext == ".cpp" || ext == ".cs") {
        return AssetItemType::Script;
    }
    if (ext == ".zscene" || ext == ".json" || ext == ".map") {
        return AssetItemType::Level;
    }
    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg") {
        return AssetItemType::Audio;
    }
    if (ext == ".hlsl" || ext == ".zeshader" || ext == ".vfx") {
        return AssetItemType::VFX;
    }
    return AssetItemType::Unknown;
}

void AssetRegistry::ScanProjectFolder(const std::string& folderPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    m_RootFolder.name = "Blueman Cooked Assets";
    m_RootFolder.path = folderPath;
    m_RootFolder.items.clear();
    m_RootFolder.subfolders.clear();

    std::error_code ec;
    if (!fs::exists(folderPath, ec)) {
        fs::create_directories(folderPath, ec);
    }

    if (!fs::exists(folderPath, ec) || !fs::is_directory(folderPath, ec)) {
        return;
    }

    for (const auto& entry : fs::directory_iterator(folderPath, ec)) {
        if (ec) break;
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            AssetItem item;
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.type = DetectItemType(ext);
            m_RootFolder.items.push_back(item);
        } else if (entry.is_directory()) {
            AssetFolder subFolder;
            subFolder.name = entry.path().filename().string();
            subFolder.path = entry.path().string();

            for (const auto& subEntry : fs::directory_iterator(entry.path(), ec)) {
                if (subEntry.is_regular_file()) {
                    std::string ext = subEntry.path().extension().string();
                    AssetItem subItem;
                    subItem.name = subEntry.path().filename().string();
                    subItem.path = subEntry.path().string();
                    subItem.type = DetectItemType(ext);
                    subFolder.items.push_back(subItem);
                }
            }
            m_RootFolder.subfolders.push_back(subFolder);
        }
    }
}

void AssetRegistry::RegisterAsset(const AssetItem& item, const std::string& folderPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check if asset already exists in root folder to avoid duplicate entries
    for (auto& existing : m_RootFolder.items) {
        if (existing.path == item.path || existing.name == item.name) {
            existing = item;
            return;
        }
    }

    if (folderPath.empty() || folderPath == m_RootFolder.path) {
        m_RootFolder.items.push_back(item);
        return;
    }

    AssetFolder* folder = const_cast<AssetFolder*>(FindFolder(folderPath, &m_RootFolder));
    if (folder) {
        for (auto& existing : folder->items) {
            if (existing.path == item.path || existing.name == item.name) {
                existing = item;
                return;
            }
        }
        folder->items.push_back(item);
    } else {
        m_RootFolder.items.push_back(item);
    }
}

void AssetRegistry::RegisterFolder(const AssetFolder& folder) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_RootFolder.subfolders.push_back(folder);
}

void AssetRegistry::Clear() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_RootFolder.items.clear();
    m_RootFolder.subfolders.clear();
}

void AssetRegistry::SetRootFolder(const AssetFolder& folder) {
    std::lock_guard<std::mutex> lock(m_Mutex);
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
