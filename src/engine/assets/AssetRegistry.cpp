#include "engine/assets/AssetRegistry.h"
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
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds" || ext == ".bmp" || ext == ".hdr" || ext == ".exr" || ext == ".ztex" || ext == ".raw" || ext == ".r16" || ext == ".r8") {
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

bool AssetRegistry::IsDependencyOnlyExtension(const std::string& extension) {
    std::string ext = extension;
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });

    // Known dependency/sidecar formats (e.g. .bin glTF buffers) that should not be loaded standalone
    if (ext.rfind(".bin", 0) == 0) {
        return true;
    }
    return false;
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
            if (IsDependencyOnlyExtension(ext)) {
                continue; // Exclude sidecar dependency files (e.g., .bin) from browsable asset list
            }
            AssetItem item;
            item.name = entry.path().filename().string();
            item.path = entry.path().string();
            item.type = DetectItemType(ext);
            item.isDependencyOnly = false;
            m_RootFolder.items.push_back(item);
        } else if (entry.is_directory()) {
            AssetFolder subFolder;
            subFolder.name = entry.path().filename().string();
            subFolder.path = entry.path().string();

            for (const auto& subEntry : fs::directory_iterator(entry.path(), ec)) {
                if (subEntry.is_regular_file()) {
                    std::string ext = subEntry.path().extension().string();
                    if (IsDependencyOnlyExtension(ext)) {
                        continue; // Exclude sidecar dependency files (e.g., .bin) from subfolders
                    }
                    AssetItem subItem;
                    subItem.name = subEntry.path().filename().string();
                    subItem.path = subEntry.path().string();
                    subItem.type = DetectItemType(ext);
                    subItem.isDependencyOnly = false;
                    subFolder.items.push_back(subItem);
                }
            }
            m_RootFolder.subfolders.push_back(subFolder);
        }
    }
}

void AssetRegistry::RegisterAsset(const AssetItem& item, const std::string& folderPath) {
    std::lock_guard<std::mutex> lock(m_Mutex);

    std::string ext = fs::path(item.path).extension().string();
    if (item.isDependencyOnly || IsDependencyOnlyExtension(ext)) {
        return; // Exclude sidecar dependency files from standalone registration
    }

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

} // namespace EngineEditor
