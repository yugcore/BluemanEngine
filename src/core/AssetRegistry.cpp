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
    audioFolder.items = {
        { "A_Ambient_Wind_Loop", AssetItemType::Audio, audioFolder.path + "/A_Ambient_Wind_Loop" },
        { "A_Explosion_Heavy", AssetItemType::Audio, audioFolder.path + "/A_Explosion_Heavy" },
        { "A_Footstep_Concrete", AssetItemType::Audio, audioFolder.path + "/A_Footstep_Concrete" }
    };

    // 4. Scripts (Zelyn & C++)
    AssetFolder scriptsFolder;
    scriptsFolder.name = "Scripts";
    scriptsFolder.path = "ZeGFX Workspace/Content/StarterContent/Scripts";
    scriptsFolder.items = {
        { "PlayerController.zyn", AssetItemType::Script, scriptsFolder.path + "/PlayerController.zyn" },
        { "CameraRig.zl", AssetItemType::Script, scriptsFolder.path + "/CameraRig.zl" },
        { "InventorySystem.cpp", AssetItemType::Script, scriptsFolder.path + "/InventorySystem.cpp" }
    };

    // 5. HDRI & Textures
    AssetFolder hdriFolder;
    hdriFolder.name = "HDRI";
    hdriFolder.path = "ZeGFX Workspace/Content/StarterContent/HDRI";

    // 6. Maps / Levels
    AssetFolder mapsFolder;
    mapsFolder.name = "Maps";
    mapsFolder.path = "ZeGFX Workspace/Content/StarterContent/Maps";
    mapsFolder.items = {
        { "L_Main_Showcase", AssetItemType::Level, mapsFolder.path + "/L_Main_Showcase" },
        { "L_Lighting_Studio", AssetItemType::Level, mapsFolder.path + "/L_Lighting_Studio" }
    };

    // 7. Particles & VFX
    AssetFolder particlesFolder;
    particlesFolder.name = "Particles";
    particlesFolder.path = "ZeGFX Workspace/Content/StarterContent/Particles";
    particlesFolder.items = {
        { "P_Fire_Sparks", AssetItemType::VFX, particlesFolder.path + "/P_Fire_Sparks" },
        { "P_Smoke_Dense", AssetItemType::VFX, particlesFolder.path + "/P_Smoke_Dense" }
    };

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
    texturesFolder.items = {
        { "T_Concrete_Normal_2K", AssetItemType::Texture, texturesFolder.path + "/T_Concrete_Normal_2K" },
        { "T_Brick_Albedo_4K", AssetItemType::Texture, texturesFolder.path + "/T_Brick_Albedo_4K" },
        { "T_Skybox_HDRI_Cubemap", AssetItemType::Texture, texturesFolder.path + "/T_Skybox_HDRI_Cubemap" }
    };

    starterFolder.subfolders = {
        archFolder,
        audioFolder,
        scriptsFolder,
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
