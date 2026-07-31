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

    // --- Content Folder ---
    AssetFolder contentFolder;
    contentFolder.name = "Content";
    contentFolder.path = "ZeGFX Workspace/Content";

    // --- StarterContent Folder ---
    AssetFolder starterFolder;
    starterFolder.name = "StarterContent";
    starterFolder.path = "ZeGFX Workspace/Content/StarterContent";

    // 1. Materials Folder (matching UE5 screenshot)
    AssetFolder materialsFolder;
    materialsFolder.name = "Materials";
    materialsFolder.path = "ZeGFX Workspace/Content/StarterContent/Materials";
    materialsFolder.items = {
        { "M_Asset_Platform", AssetItemType::Material, materialsFolder.path + "/M_Asset_Platform" },
        { "M_Basic_Floor", AssetItemType::Material, materialsFolder.path + "/M_Basic_Floor" },
        { "M_Basic_Wall", AssetItemType::Material, materialsFolder.path + "/M_Basic_Wall" },
        { "M_Brick_Clay_Beveled", AssetItemType::Material, materialsFolder.path + "/M_Brick_Clay_Beveled" },
        { "M_Brick_Clay_Old", AssetItemType::Material, materialsFolder.path + "/M_Brick_Clay_Old" },
        { "M_Brick_Cut_Stone", AssetItemType::Material, materialsFolder.path + "/M_Brick_Cut_Stone" },
        { "M_Brick_Hewn_Stone", AssetItemType::Material, materialsFolder.path + "/M_Brick_Hewn_Stone" },
        { "M_Ceramic_Tile_Checker", AssetItemType::Material, materialsFolder.path + "/M_Ceramic_Tile_Checker" },
        { "M_CobbleStone_Pebble", AssetItemType::Material, materialsFolder.path + "/M_CobbleStone_Pebble" },
        { "M_CobbleStone_Rough", AssetItemType::Material, materialsFolder.path + "/M_CobbleStone_Rough" },
        { "M_CobbleStone_Smooth", AssetItemType::Material, materialsFolder.path + "/M_CobbleStone_Smooth" },
        { "M_ColorGrid_LowSpec", AssetItemType::Material, materialsFolder.path + "/M_ColorGrid_LowSpec" },
        { "M_Concrete_Grime", AssetItemType::Material, materialsFolder.path + "/M_Concrete_Grime" },
        { "M_Concrete_Panels", AssetItemType::Material, materialsFolder.path + "/M_Concrete_Panels" },
        { "M_Concrete_Poured", AssetItemType::Material, materialsFolder.path + "/M_Concrete_Poured" },
        { "M_Concrete_Tiles", AssetItemType::Material, materialsFolder.path + "/M_Concrete_Tiles" },
        { "M_Glass", AssetItemType::Material, materialsFolder.path + "/M_Glass" },
        { "M_Ground_Grass", AssetItemType::Material, materialsFolder.path + "/M_Ground_Grass" },
        { "M_Ground_Gravel", AssetItemType::Material, materialsFolder.path + "/M_Ground_Gravel" },
        { "M_Ground_Moss", AssetItemType::Material, materialsFolder.path + "/M_Ground_Moss" },
        { "M_Metal_Brushed_Nickel", AssetItemType::Material, materialsFolder.path + "/M_Metal_Brushed_Nickel" },
        { "M_Metal_Burnished_Steel", AssetItemType::Material, materialsFolder.path + "/M_Metal_Burnished_Steel" },
        { "M_Metal_Chrome", AssetItemType::Material, materialsFolder.path + "/M_Metal_Chrome" },
        { "M_Metal_Copper", AssetItemType::Material, materialsFolder.path + "/M_Metal_Copper" },
        { "M_Metal_Gold", AssetItemType::Material, materialsFolder.path + "/M_Metal_Gold" },
        { "M_Metal_Rust", AssetItemType::Material, materialsFolder.path + "/M_Metal_Rust" },
        { "M_Metal_Steel", AssetItemType::Material, materialsFolder.path + "/M_Metal_Steel" },
        { "M_Rock_Basalt", AssetItemType::Material, materialsFolder.path + "/M_Rock_Basalt" },
        { "M_Rock_Marble_Polished", AssetItemType::Material, materialsFolder.path + "/M_Rock_Marble_Polished" },
        { "M_Rock_Sandstone", AssetItemType::Material, materialsFolder.path + "/M_Rock_Sandstone" },
        { "M_Rock_Slate", AssetItemType::Material, materialsFolder.path + "/M_Rock_Slate" }
    };

    // 2. Architecture
    AssetFolder archFolder;
    archFolder.name = "Architecture";
    archFolder.path = "ZeGFX Workspace/Content/StarterContent/Architecture";
    archFolder.items = {
        { "SM_DoorFrame", AssetItemType::Mesh, archFolder.path + "/SM_DoorFrame" },
        { "SM_PillarFrame", AssetItemType::Mesh, archFolder.path + "/SM_PillarFrame" },
        { "SM_Wall_400x400", AssetItemType::Mesh, archFolder.path + "/SM_Wall_400x400" }
    };

    // 3. Audio
    AssetFolder audioFolder;
    audioFolder.name = "Audio";
    audioFolder.path = "ZeGFX Workspace/Content/StarterContent/Audio";

    // 4. Blueprints
    AssetFolder blueprintsFolder;
    blueprintsFolder.name = "Blueprints";
    blueprintsFolder.path = "ZeGFX Workspace/Content/StarterContent/Blueprints";
    blueprintsFolder.items = {
        { "BP_GameController", AssetItemType::Blueprint, blueprintsFolder.path + "/BP_GameController" },
        { "BP_LightStudio", AssetItemType::Blueprint, blueprintsFolder.path + "/BP_LightStudio" }
    };

    // 5. HDRI
    AssetFolder hdriFolder;
    hdriFolder.name = "HDRI";
    hdriFolder.path = "ZeGFX Workspace/Content/StarterContent/HDRI";

    // 6. Maps
    AssetFolder mapsFolder;
    mapsFolder.name = "Maps";
    mapsFolder.path = "ZeGFX Workspace/Content/StarterContent/Maps";

    // 7. Particles
    AssetFolder particlesFolder;
    particlesFolder.name = "Particles";
    particlesFolder.path = "ZeGFX Workspace/Content/StarterContent/Particles";

    // 8. Props
    AssetFolder propsFolder;
    propsFolder.name = "Props";
    propsFolder.path = "ZeGFX Workspace/Content/StarterContent/Props";

    // 9. Shapes
    AssetFolder shapesFolder;
    shapesFolder.name = "Shapes";
    shapesFolder.path = "ZeGFX Workspace/Content/StarterContent/Shapes";

    // 10. Textures
    AssetFolder texturesFolder;
    texturesFolder.name = "Textures";
    texturesFolder.path = "ZeGFX Workspace/Content/StarterContent/Textures";

    starterFolder.subfolders = {
        archFolder,
        audioFolder,
        blueprintsFolder,
        hdriFolder,
        mapsFolder,
        materialsFolder,
        particlesFolder,
        propsFolder,
        shapesFolder,
        texturesFolder
    };

    contentFolder.subfolders = { starterFolder };

    // Engine folder
    AssetFolder engineFolder;
    engineFolder.name = "Engine";
    engineFolder.path = "ZeGFX Workspace/Engine";

    m_RootFolder.subfolders = { contentFolder, engineFolder };
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
        case AssetItemType::Material:    return ImVec4(0.25f, 0.80f, 0.40f, 1.00f); // Green (UE5 Material)
        case AssetItemType::Texture:     return ImVec4(0.95f, 0.75f, 0.20f, 1.00f); // Amber/Yellow
        case AssetItemType::Blueprint:   return ImVec4(0.65f, 0.40f, 0.90f, 1.00f); // Purple
        case AssetItemType::AI:          return ImVec4(0.95f, 0.50f, 0.20f, 1.00f); // Orange
        case AssetItemType::LevelScript: return ImVec4(0.90f, 0.30f, 0.50f, 1.00f); // Pink
        default:                         return ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    }
}

} // namespace EngineEditor
