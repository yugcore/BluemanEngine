#include "SceneGraph.h"
#include "third_party/IconsFontAwesome6.h"

namespace EngineEditor {

SceneGraph& SceneGraph::Get() {
    static SceneGraph instance;
    return instance;
}

SceneGraph::SceneGraph() {
    SeedDummyData();
}

void SceneGraph::SeedDummyData() {
    // 1. Environments
    SceneNode envFolder;
    envFolder.name = "Environments";
    envFolder.type = SceneNodeType::Folder;
    envFolder.world = "DefaultWorld";
    envFolder.panel = "LevelPanel";

    SceneNode villageFolder;
    villageFolder.name = "Modular_Village";
    villageFolder.type = SceneNodeType::Folder;
    villageFolder.world = "DefaultWorld";
    villageFolder.panel = "LevelPanel";

    SceneNode bridgesFolder;
    bridgesFolder.name = "Bridges";
    bridgesFolder.type = SceneNodeType::Folder;
    bridgesFolder.world = "DefaultWorld";
    bridgesFolder.panel = "LevelPanel";

    SceneNode buildingsFolder;
    buildingsFolder.name = "Buildings_01";
    buildingsFolder.type = SceneNodeType::Folder;
    buildingsFolder.world = "DefaultWorld";
    buildingsFolder.panel = "LevelPanel";

    villageFolder.children = { bridgesFolder, buildingsFolder };

    SceneNode skyFolder;
    skyFolder.name = "Sky";
    skyFolder.type = SceneNodeType::Folder;
    skyFolder.world = "DefaultWorld";
    skyFolder.panel = "SkyPanel";

    SceneNode skyAtmosphere;
    skyAtmosphere.name = "SkyAtmosphere";
    skyAtmosphere.type = SceneNodeType::SkyAtmosphere;
    skyAtmosphere.world = "DefaultWorld";
    skyAtmosphere.panel = "SkyPanel";

    SceneNode sunLight;
    sunLight.name = "SunLight";
    sunLight.type = SceneNodeType::Light;
    sunLight.world = "DefaultWorld";
    sunLight.panel = "LightPanel";

    SceneNode skyLight;
    skyLight.name = "SkyLight";
    skyLight.type = SceneNodeType::Light;
    skyLight.world = "DefaultWorld";
    skyLight.panel = "LightPanel";

    skyFolder.children = { skyAtmosphere, sunLight, skyLight };

    envFolder.children = { villageFolder, skyFolder };

    // 2. Player
    SceneNode playerFolder;
    playerFolder.name = "Player";
    playerFolder.type = SceneNodeType::Folder;
    playerFolder.world = "GameWorld";
    playerFolder.panel = "ActorPanel";

    SceneNode charActor;
    charActor.name = "CharacterActor";
    charActor.type = SceneNodeType::Actor;
    charActor.world = "GameWorld";
    charActor.panel = "ActorPanel";

    SceneNode cameraActor;
    cameraActor.name = "CameraActor";
    cameraActor.type = SceneNodeType::Camera;
    cameraActor.world = "GameWorld";
    cameraActor.panel = "CameraPanel";

    playerFolder.children = { charActor, cameraActor };

    // 3. Audio
    SceneNode audioFolder;
    audioFolder.name = "Audio";
    audioFolder.type = SceneNodeType::Folder;
    audioFolder.world = "AudioWorld";
    audioFolder.panel = "SoundPanel";

    SceneNode ambientSound;
    ambientSound.name = "AmbientSoundVolume";
    ambientSound.type = SceneNodeType::Audio;
    ambientSound.world = "AudioWorld";
    ambientSound.panel = "SoundPanel";

    audioFolder.children = { ambientSound };

    m_RootNodes = { envFolder, playerFolder, audioFolder };
}

const SceneNode* SceneGraph::FindNode(const std::string& name, const std::vector<SceneNode>* nodes) const {
    if (nodes == nullptr) {
        nodes = &m_RootNodes;
    }
    for (const auto& node : *nodes) {
        if (node.name == name) return &node;
        const SceneNode* found = FindNode(name, &node.children);
        if (found) return found;
    }
    return nullptr;
}

const char* SceneGraph::GetTypeName(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return "Folder";
        case SceneNodeType::Actor:         return "Actor";
        case SceneNodeType::Light:         return "Light";
        case SceneNodeType::Camera:        return "Camera";
        case SceneNodeType::Audio:         return "Audio";
        case SceneNodeType::SkyAtmosphere: return "SkyAtmosphere";
        case SceneNodeType::Component:     return "Component";
        default:                           return "Object";
    }
}

ImVec4 SceneGraph::GetTypeColor(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return ImVec4(0.85f, 0.85f, 0.85f, 1.00f); // Light Gray
        case SceneNodeType::Actor:         return ImVec4(0.30f, 0.75f, 0.95f, 1.00f); // Cyan
        case SceneNodeType::Light:         return ImVec4(0.95f, 0.80f, 0.25f, 1.00f); // Amber Yellow
        case SceneNodeType::Camera:        return ImVec4(0.40f, 0.85f, 0.50f, 1.00f); // Green
        case SceneNodeType::Audio:         return ImVec4(0.90f, 0.45f, 0.25f, 1.00f); // Orange
        case SceneNodeType::SkyAtmosphere: return ImVec4(0.70f, 0.45f, 0.95f, 1.00f); // Purple
        case SceneNodeType::Component:     return ImVec4(0.60f, 0.60f, 0.60f, 1.00f); // Muted
        default:                           return ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    }
}

const char* SceneGraph::GetTypeIconTag(SceneNodeType type) {
    switch (type) {
        case SceneNodeType::Folder:        return ICON_FA_FOLDER;
        case SceneNodeType::Actor:         return ICON_FA_CUBE;
        case SceneNodeType::Light:         return ICON_FA_LIGHTBULB;
        case SceneNodeType::Camera:        return ICON_FA_CAMERA;
        case SceneNodeType::Audio:         return ICON_FA_VOLUME_HIGH;
        case SceneNodeType::SkyAtmosphere: return ICON_FA_SUN;
        case SceneNodeType::Component:     return ICON_FA_SLIDERS;
        default:                           return ICON_FA_CUBE;
    }
}

} // namespace EngineEditor
