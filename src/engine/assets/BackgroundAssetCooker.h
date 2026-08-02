#ifndef BACKGROUND_ASSET_COOKER_H
#define BACKGROUND_ASSET_COOKER_H

#include "engine/assets/AssetRegistry.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

namespace EngineEditor {

struct CookedAssetResult {
    std::string sourcePath;
    std::string outputCachePath;
    std::vector<std::string> textureRefs;
    AssetItemType assetType = AssetItemType::Unknown;
    uint32_t renderMeshHandle = 0;
    bool success = false;
    std::string errorMsg = "";
    float spawnPos[3] = { 0.0f, 0.0f, 0.0f };
    bool hasSpawnPos = false;
};

struct PendingCookRequest {
    std::string filePath;
    float spawnPos[3] = { 0.0f, 0.0f, 0.0f };
    bool hasSpawnPos = false;
};

struct CookTask {
    std::string sourcePath;
    std::string fileName;
    std::string assetType;
    float progress = 0.0f;
    std::string statusText = "Initializing...";
    bool isDone = false;
    bool hasError = false;
    std::string errorMsg = "";
};

struct NotificationMsg {
    std::string text;
    bool isSuccess = true;
    std::chrono::steady_clock::time_point startTime;
    float durationSeconds = 4.0f;
};

struct CookingStatus {
    bool isCooking = false;
    float currentProgress = 0.0f;
    std::string currentTaskName;
    std::string currentStatusText;
    std::vector<NotificationMsg> notifications;
};

class BackgroundAssetCooker {
public:
    static BackgroundAssetCooker& Get();

    BackgroundAssetCooker();
    ~BackgroundAssetCooker();

    // Queue one or multiple files for background cooking & importing
    void QueueFileForCooking(const std::string& filePath);
    void QueueFileForCooking(const std::string& filePath, float spawnX, float spawnY, float spawnZ);
    void QueueFilesForCooking(const std::vector<std::string>& filePaths);

    // Updates state & dispatches main-thread completion callbacks
    void Update();

    // Main-thread thread-safe retrieval of completed cooking results
    bool PopCompletedResult(CookedAssetResult& outResult);
    bool HasCompletedResults() const;

    CookingStatus GetCookingStatus() const;

    bool IsCooking() const { return m_IsCooking.load(); }
    float GetCurrentProgress() const { return m_CurrentProgress.load(); }
    std::string GetCurrentTaskName() const { std::lock_guard<std::mutex> lock(m_Mutex); return m_CurrentTaskName; }
    std::string GetCurrentStatusText() const { std::lock_guard<std::mutex> lock(m_Mutex); return m_CurrentStatusText; }

private:
    void WorkerLoop();
    void ProcessSingleFile(const PendingCookRequest& request);

    mutable std::mutex m_Mutex;
    std::queue<PendingCookRequest> m_PendingQueue;
    std::queue<CookedAssetResult> m_CompletedResults;
    std::thread m_WorkerThread;
    std::atomic<bool> m_StopWorker{ false };
    std::atomic<bool> m_IsCooking{ false };
    std::atomic<float> m_CurrentProgress{ 0.0f };

    std::string m_CurrentTaskName = "";
    std::string m_CurrentStatusText = "";

    std::vector<NotificationMsg> m_Notifications;
    std::vector<CookTask> m_CompletedTasks;
};

} // namespace EngineEditor

#endif // BACKGROUND_ASSET_COOKER_H
