#include "engine/assets/BackgroundAssetCooker.h"
#include "engine/assets/AssetRegistry.h"
#include "engine/core/Logger.h"
#include "engine/scene/SceneGraph.h"
#include "render/ZeGFXAdapter.h"
#include "asset_importer.h"
#include "cooker/asset_cooker.h"

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <cctype>

namespace EngineEditor {

BackgroundAssetCooker& BackgroundAssetCooker::Get() {
    static BackgroundAssetCooker instance;
    return instance;
}

BackgroundAssetCooker::BackgroundAssetCooker() {
    m_WorkerThread = std::thread(&BackgroundAssetCooker::WorkerLoop, this);
}

BackgroundAssetCooker::~BackgroundAssetCooker() {
    m_StopWorker.store(true);
    if (m_WorkerThread.joinable()) {
        m_WorkerThread.join();
    }
}

void BackgroundAssetCooker::QueueFileForCooking(const std::string& filePath) {
    if (filePath.empty()) return;
    std::lock_guard<std::mutex> lock(m_Mutex);
    PendingCookRequest req;
    req.filePath = filePath;
    req.hasSpawnPos = false;
    m_PendingQueue.push(req);
    Logger::Get().Info("[AssetCooker] Queued asset for background cooking: " + filePath);
}

void BackgroundAssetCooker::QueueFileForCooking(const std::string& filePath, float spawnX, float spawnY, float spawnZ) {
    if (filePath.empty()) return;
    std::lock_guard<std::mutex> lock(m_Mutex);
    PendingCookRequest req;
    req.filePath = filePath;
    req.spawnPos[0] = spawnX;
    req.spawnPos[1] = spawnY;
    req.spawnPos[2] = spawnZ;
    req.hasSpawnPos = true;
    m_PendingQueue.push(req);
    Logger::Get().Info("[AssetCooker] Queued asset for background cooking with spawn position: " + filePath);
}

void BackgroundAssetCooker::QueueFilesForCooking(const std::vector<std::string>& filePaths) {
    for (const auto& path : filePaths) {
        QueueFileForCooking(path);
    }
}

void BackgroundAssetCooker::WorkerLoop() {
    while (!m_StopWorker.load()) {
        PendingCookRequest targetReq;
        bool hasTask = false;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_PendingQueue.empty()) {
                targetReq = m_PendingQueue.front();
                m_PendingQueue.pop();
                hasTask = true;
            }
        }

        if (hasTask && !targetReq.filePath.empty()) {
            m_IsCooking.store(true);
            ProcessSingleFile(targetReq);
            m_IsCooking.store(false);
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

static std::string GetFileExtensionLower(const std::string& path) {
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return "";
    std::string ext = path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return (char)std::tolower(c); });
    return ext;
}

bool BackgroundAssetCooker::PopCompletedResult(CookedAssetResult& outResult) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_CompletedResults.empty()) return false;
    outResult = m_CompletedResults.front();
    m_CompletedResults.pop();
    return true;
}

bool BackgroundAssetCooker::HasCompletedResults() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    return !m_CompletedResults.empty();
}

void BackgroundAssetCooker::ProcessSingleFile(const PendingCookRequest& request) {
    const std::string& filePath = request.filePath;
    namespace fs = std::filesystem;
    std::string fileName = fs::path(filePath).filename().string();
    std::string ext = GetFileExtensionLower(filePath);

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_CurrentTaskName = fileName;
        m_CurrentStatusText = "Analyzing source structure...";
    }
    m_CurrentProgress.store(0.10f);

    AssetItemType itemType = AssetItemType::Unknown;
    bool success = true;
    std::string statusMsg = "";
    std::string targetPath = "";
    std::string cookedOutPath = "";

    const std::string projectDir = "CookedAssets";
    std::error_code dirEc;
    fs::create_directories(projectDir, dirEc);

    if (ext == ".fbx" || ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".vox" || ext == ".zmesh") {
        itemType = AssetItemType::Mesh;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_CurrentStatusText = "Cooking 3D Mesh Geometry payload...";
        }
        m_CurrentProgress.store(0.35f);

        std::string stem = fs::path(filePath).stem().string();
        cookedOutPath = projectDir + "\\" + stem + ".zmesh";

        zegfx::cooker::AssetCooker cooker;
        bool cookOk = cooker.CookMesh(filePath, cookedOutPath);
        if (cookOk) {
            statusMsg = "Successfully cooked geometry payload to " + cookedOutPath;
            success = true;
        } else {
            statusMsg = "Cooker fallback: copying raw mesh payload to project cache";
            std::error_code ec;
            fs::path dest = fs::path(projectDir) / fileName;
            fs::copy_file(filePath, dest, fs::copy_options::overwrite_existing, ec);
            targetPath = dest.string();
            cookedOutPath = targetPath;
            success = true;
        }
    } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds" || ext == ".hdr" || ext == ".ztex") {
        itemType = AssetItemType::Texture;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_CurrentStatusText = "Compressing texture payload...";
        }
        m_CurrentProgress.store(0.50f);

        std::error_code ec;
        fs::path dest = fs::path(projectDir) / fileName;
        fs::copy_file(filePath, dest, fs::copy_options::overwrite_existing, ec);
        targetPath = dest.string();
        cookedOutPath = targetPath;
        statusMsg = "Imported Texture2D asset payload";
        success = true;
    } else if (ext == ".zmat" || ext == ".mat" || ext == ".material") {
        itemType = AssetItemType::Material;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_CurrentStatusText = "Parsing PBR Material descriptor...";
        }
        m_CurrentProgress.store(0.60f);

        std::error_code ec;
        fs::path dest = fs::path(projectDir) / fileName;
        fs::copy_file(filePath, dest, fs::copy_options::overwrite_existing, ec);
        targetPath = dest.string();
        cookedOutPath = targetPath;
        statusMsg = "Imported Material asset payload";
        success = true;
    } else {
        itemType = AssetRegistry::DetectItemType(ext);
        std::error_code ec;
        fs::path dest = fs::path(projectDir) / fileName;
        fs::copy_file(filePath, dest, fs::copy_options::overwrite_existing, ec);
        targetPath = dest.string();
        cookedOutPath = targetPath;
        statusMsg = "Imported asset file payload";
        success = true;
    }

    m_CurrentProgress.store(0.85f);

    // Copy sibling .bin buffers (essential for glTF assets)
    std::string stemName = fs::path(filePath).stem().string();
    fs::path parentDir = fs::path(filePath).parent_path();
    fs::path binSibling = parentDir / (stemName + ".bin");
    if (fs::exists(binSibling)) {
        std::error_code binEc;
        fs::copy_file(binSibling, fs::path(projectDir) / (stemName + ".bin"), fs::copy_options::overwrite_existing, binEc);
    }

    // Thread-safe dispatch to AssetRegistry & full rescan
    AssetItem item;
    item.name = fileName;
    item.type = itemType;
    item.path = targetPath.empty() ? filePath : targetPath;

    AssetRegistry::Get().RegisterAsset(item, projectDir);
    AssetRegistry::Get().ScanProjectFolder(projectDir);

    // Add completion notification and result queue entry
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        NotificationMsg notif;
        notif.text = "Imported " + fileName + (success ? " successfully!" : " with errors!");
        notif.isSuccess = success;
        notif.startTime = std::chrono::steady_clock::now();
        notif.durationSeconds = 4.5f;
        m_Notifications.push_back(notif);

        CookedAssetResult resultRecord;
        resultRecord.sourcePath = filePath;
        resultRecord.outputCachePath = cookedOutPath.empty() ? targetPath : cookedOutPath;
        resultRecord.assetType = itemType;
        resultRecord.success = success;
        resultRecord.errorMsg = success ? "" : statusMsg;
        resultRecord.spawnPos[0] = request.spawnPos[0];
        resultRecord.spawnPos[1] = request.spawnPos[1];
        resultRecord.spawnPos[2] = request.spawnPos[2];
        resultRecord.hasSpawnPos = request.hasSpawnPos;
        m_CompletedResults.push(resultRecord);

        m_CurrentTaskName = "";
        m_CurrentStatusText = "";
    }

    Logger::Get().Info("[AssetCooker] " + statusMsg + " -> " + fileName);
}

void BackgroundAssetCooker::Update() {
    // 1. Clean up expired notifications
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto now = std::chrono::steady_clock::now();
        m_Notifications.erase(
            std::remove_if(m_Notifications.begin(), m_Notifications.end(), [&](const NotificationMsg& n) {
                float elapsed = std::chrono::duration<float>(now - n.startTime).count();
                return elapsed >= n.durationSeconds;
            }),
            m_Notifications.end()
        );
    }

    // 2. Main-thread processing of completed asset cooking results
    CookedAssetResult result;
    while (PopCompletedResult(result)) {
        if (result.success && result.assetType == AssetItemType::Mesh) {
            // Load GPU mesh handle into ZeGFXAdapter
            fprintf(stderr, "[AssetCooker::Update] Loading mesh into GPU: '%s'\n", result.outputCachePath.c_str());
            auto meshHandle = ZeGFXAdapter::Get().LoadMeshAsset(result.outputCachePath);
            fprintf(stderr, "[AssetCooker::Update] LoadMeshAsset result: handle.valid()=%d\n", (int)meshHandle.valid());

            // Construct new SceneNode for the imported mesh
            namespace fs = std::filesystem;
            std::string baseName = fs::path(result.sourcePath).stem().string();

            SceneNode newNode;
            newNode.id = SceneGraph::Get().GenerateNodeId();
            newNode.name = baseName;
            newNode.type = SceneNodeType::Actor;
            newNode.meshPath = result.outputCachePath;

            // Auto-assign matching .zmat material if present in same directory
            std::filesystem::path matPath = result.outputCachePath;
            matPath.replace_extension(".zmat");
            if (std::filesystem::exists(matPath)) {
                newNode.materialPath = matPath.string();
            }

            if (result.hasSpawnPos) {
                newNode.location[0] = result.spawnPos[0];
                newNode.location[1] = result.spawnPos[1];
                newNode.location[2] = result.spawnPos[2];
            } else {
                newNode.location[0] = 0.0f;
                newNode.location[1] = 0.0f;
                newNode.location[2] = 0.0f;
            }

            // Add node to SceneGraph
            SceneGraph::Get().AddNode(newNode);

            Logger::Get().Info("[AssetCooker] Automatically instantiated SceneNode '" + newNode.name + "' with mesh " + result.outputCachePath);
        }
    }
}

CookingStatus BackgroundAssetCooker::GetCookingStatus() const {
    std::lock_guard<std::mutex> lock(m_Mutex);
    CookingStatus status;
    status.isCooking = m_IsCooking.load();
    status.currentProgress = m_CurrentProgress.load();
    status.currentTaskName = m_CurrentTaskName;
    status.currentStatusText = m_CurrentStatusText;
    status.notifications = m_Notifications;
    return status;
}

} // namespace EngineEditor
