#include "AssetRegistry.h"
#include "third_party/IconsFontAwesome6.h"
#include <algorithm>

namespace EngineEditor {

AssetRegistry& AssetRegistry::Get() {
    static AssetRegistry instance;
    return instance;
}

AssetRegistry::AssetRegistry() {
    SeedDummyData();
}

void AssetRegistry::SeedDummyData() {
    m_RootFolder.name = "ZeGFX Workspace";
    m_RootFolder.path = "ZeGFX Workspace";

    AssetFolder cookedFolder;
    cookedFolder.name = "Blueman Cooked Assets";
    cookedFolder.path = "ZeGFX Workspace/Blueman Cooked Assets";

    // 1. Meshes
    AssetFolder meshesFolder;
    meshesFolder.name = "Meshes";
    meshesFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/Meshes";
    meshesFolder.items = {
        { "M_Village_Wall_01_Base", AssetItemType::Mesh, meshesFolder.path + "/M_Village_Wall_01_Base" },
        { "M_Village_Gate_Detailed", AssetItemType::Mesh, meshesFolder.path + "/M_Village_Gate_Detailed" },
        { "M_Village_Ground_Terrain_H", AssetItemType::Mesh, meshesFolder.path + "/M_Village_Ground_Terrain_H" }
    };

    // 2. Materials
    AssetFolder materialsFolder;
    materialsFolder.name = "Materials";
    materialsFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/Materials";
    materialsFolder.items = {
        { "Mat_Village_Architecture_PBR", AssetItemType::Material, materialsFolder.path + "/Mat_Village_Architecture_PBR" },
        { "Mat_Village_Terrain_Auto", AssetItemType::Material, materialsFolder.path + "/Mat_Village_Terrain_Auto" },
        { "Mat_Sky_Atmosphere_Advanced", AssetItemType::Material, materialsFolder.path + "/Mat_Sky_Atmosphere_Advanced" }
    };

    // 3. Textures
    AssetFolder texturesFolder;
    texturesFolder.name = "Textures";
    texturesFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/Textures";
    texturesFolder.items = {
        { "T_Village_Wall_Albedo", AssetItemType::Texture, texturesFolder.path + "/T_Village_Wall_Albedo" },
        { "T_Village_Wall_Normal", AssetItemType::Texture, texturesFolder.path + "/T_Village_Wall_Normal" },
        { "T_Village_Wall_Roughness", AssetItemType::Texture, texturesFolder.path + "/T_Village_Wall_Roughness" }
    };

    // 4. Blueprints
    AssetFolder blueprintsFolder;
    blueprintsFolder.name = "Blueprints";
    blueprintsFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/Blueprints";
    blueprintsFolder.items = {
        { "BP_GameController", AssetItemType::Blueprint, blueprintsFolder.path + "/BP_GameController" },
        { "BP_Npc_Villager", AssetItemType::Blueprint, blueprintsFolder.path + "/BP_Npc_Villager" },
        { "BP_Npc_Villager_Havager", AssetItemType::Blueprint, blueprintsFolder.path + "/BP_Npc_Villager_Havager" }
    };

    // 5. AI
    AssetFolder aiFolder;
    aiFolder.name = "AI";
    aiFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/AI";
    aiFolder.items = {
        { "BT_Villager_Behaviors", AssetItemType::AI, aiFolder.path + "/BT_Villager_Behaviors" }
    };

    // 6. Level Scripts
    AssetFolder scriptsFolder;
    scriptsFolder.name = "Level_Scripts";
    scriptsFolder.path = "ZeGFX Workspace/Blueman Cooked Assets/Level_Scripts";
    scriptsFolder.items = {
        { "LS_Atmospheric_Sequence", AssetItemType::LevelScript, scriptsFolder.path + "/LS_Atmospheric_Sequence" }
    };

    cookedFolder.subfolders = {
        meshesFolder,
        materialsFolder,
        texturesFolder,
        blueprintsFolder,
        aiFolder,
        scriptsFolder
    };

    // Additional top-level folders matching reference
    AssetFolder sampleFolder;
    sampleFolder.name = "sampleproject";
    sampleFolder.path = "ZeGFX Workspace/sampleproject";

    AssetFolder examplesFolder;
    examplesFolder.name = "examples";
    examplesFolder.path = "ZeGFX Workspace/examples";

    AssetFolder docsFolder;
    docsFolder.name = "docs";
    docsFolder.path = "ZeGFX Workspace/docs";

    m_RootFolder.subfolders = { cookedFolder, sampleFolder, examplesFolder, docsFolder };
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
        case AssetItemType::Mesh:        return ICON_FA_CUBE " StaticMesh";
        case AssetItemType::Material:    return ICON_FA_PALETTE " Material";
        case AssetItemType::Texture:     return ICON_FA_IMAGE " Texture2D";
        case AssetItemType::Blueprint:   return ICON_FA_CODE " Blueprint";
        case AssetItemType::AI:          return ICON_FA_SLIDERS " BehaviorTree";
        case AssetItemType::LevelScript: return ICON_FA_CODE " LevelScript";
        default:                         return ICON_FA_CUBE " Asset";
    }
}

ImVec4 AssetRegistry::GetTypeColor(AssetItemType type) {
    switch (type) {
        case AssetItemType::Mesh:        return ImVec4(0.20f, 0.70f, 0.90f, 1.00f); // Cyan/Blue
        case AssetItemType::Material:    return ImVec4(0.25f, 0.80f, 0.40f, 1.00f); // Green
        case AssetItemType::Texture:     return ImVec4(0.95f, 0.75f, 0.20f, 1.00f); // Yellow/Amber
        case AssetItemType::Blueprint:   return ImVec4(0.65f, 0.40f, 0.90f, 1.00f); // Purple/Violet
        case AssetItemType::AI:          return ImVec4(0.95f, 0.50f, 0.20f, 1.00f); // Orange
        case AssetItemType::LevelScript: return ImVec4(0.90f, 0.30f, 0.50f, 1.00f); // Magenta/Pink
        default:                         return ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    }
}

} // namespace EngineEditor
