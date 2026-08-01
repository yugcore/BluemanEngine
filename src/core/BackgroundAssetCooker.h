#ifndef BACKGROUND_ASSET_COOKER_H
#define BACKGROUND_ASSET_COOKER_H

#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>

namespace EngineEditor {

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

class BackgroundAssetCooker {
public:
    static BackgroundAssetCooker& Get();

    BackgroundAssetCooker();
    ~BackgroundAssetCooker();

    // Queue one or multiple files for background cooking & importing
    void QueueFileForCooking(const std::string& filePath);
    void QueueFilesForCooking(const std::vector<std::string>& filePaths);

    // Updates state & dispatches main-thread completion callbacks
    void Update();

    // Renders small progress bar & notification overlay in bottom-right corner
    void RenderCookingOverlay();

    bool IsCooking() const { return m_IsCooking.load(); }
    float GetCurrentProgress() const { return m_CurrentProgress.load(); }
    std::string GetCurrentTaskName() const { std::lock_guard<std::mutex> lock(m_Mutex); return m_CurrentTaskName; }
    std::string GetCurrentStatusText() const { std::lock_guard<std::mutex> lock(m_Mutex); return m_CurrentStatusText; }

private:
    void WorkerLoop();
    void ProcessSingleFile(const std::string& filePath);

    mutable std::mutex m_Mutex;
    std::queue<std::string> m_PendingQueue;
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
