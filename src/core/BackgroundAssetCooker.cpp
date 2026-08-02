#include "BackgroundAssetCooker.h"
#include "AssetRegistry.h"
#include "Logger.h"
#include "EditorState.h"
#include "render/ZeGFXAdapter.h"
#include "asset_importer.h"
#include "cooker/asset_cooker.h"

#include <imgui.h>
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
    m_PendingQueue.push(filePath);
    Logger::Get().Info("[AssetCooker] Queued asset for background cooking: " + filePath);
}

void BackgroundAssetCooker::QueueFilesForCooking(const std::vector<std::string>& filePaths) {
    for (const auto& path : filePaths) {
        QueueFileForCooking(path);
    }
}

void BackgroundAssetCooker::WorkerLoop() {
    while (!m_StopWorker.load()) {
        std::string targetFile = "";
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (!m_PendingQueue.empty()) {
                targetFile = m_PendingQueue.front();
                m_PendingQueue.pop();
            }
        }

        if (!targetFile.empty()) {
            m_IsCooking.store(true);
            ProcessSingleFile(targetFile);
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

void BackgroundAssetCooker::ProcessSingleFile(const std::string& filePath) {
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
    std::string projectDir = "Z:\\Blueman Cooked Assets";
    std::error_code ec;
    fs::create_directories(projectDir, ec);
    if (ec) {
        projectDir = "./CookedAssets";
        fs::create_directories(projectDir, ec);
    }

    std::string cookedOutPath = "";

    if (ext == ".fbx" || ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".vox") {
        itemType = AssetItemType::Mesh;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_CurrentStatusText = "Cooking binary .zmesh payload via zegfx::cooker::AssetCooker...";
        }
        m_CurrentProgress.store(0.35f);

        std::string stemName = fs::path(filePath).stem().string();
        std::string outMeshPath = (fs::path(projectDir) / (stemName + ".zmesh")).string();

        zegfx::cooker::AssetCooker cooker;
        bool cookedOk = cooker.CookMesh(filePath, outMeshPath);
        m_CurrentProgress.store(0.70f);

        if (cookedOk) {
            // Verify cooked file was actually written
            std::error_code sizeEc;
            auto fileSize = fs::file_size(outMeshPath, sizeEc);
            if (sizeEc || fileSize == 0) {
                fprintf(stderr, "[AssetCooker] WARNING: CookMesh returned true but .zmesh file missing or empty: '%s' (ec=%s)\n",
                        outMeshPath.c_str(), sizeEc.message().c_str());
                statusMsg = "Mesh cooking produced empty output for " + fileName;
                success = false;
            } else {
                fprintf(stderr, "[AssetCooker] CookMesh succeeded: '%s' (%llu bytes)\n",
                        outMeshPath.c_str(), (unsigned long long)fileSize);
                statusMsg = "Mesh asset cooked successfully into binary .zmesh: " + outMeshPath;
                cookedOutPath = outMeshPath;
            }
        } else {
            fprintf(stderr, "[AssetCooker] CookMesh FAILED for '%s'\n", filePath.c_str());
            statusMsg = "Mesh cooking failed for " + fileName;
            success = false;
        }

    } else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".dds" || ext == ".bmp" || ext == ".hdr" || ext == ".exr") {
        itemType = AssetItemType::Texture;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_CurrentStatusText = "Compressing texture to BC7 block-format (.ztex)...";
        }
        m_CurrentProgress.store(0.35f);

        std::string stemName = fs::path(filePath).stem().string();
        std::string outTexPath = (fs::path(projectDir) / (stemName + ".ztex")).string();

        zegfx::cooker::AssetCooker cooker;
        bool cookedOk = cooker.CookTexture(filePath, outTexPath);
        m_CurrentProgress.store(0.70f);

        if (cookedOk) {
            statusMsg = "Texture compressed into .ztex payload successfully: " + outTexPath;
            cookedOutPath = outTexPath;
        } else {
            statusMsg = "Texture cooking failed for " + fileName;
            success = false;
        }

    } else if (ext == ".zmesh" || ext == ".zasset") {
        itemType = AssetItemType::Mesh;
        cookedOutPath = filePath;
        m_CurrentProgress.store(0.80f);
        statusMsg = "Pre-compiled ZeGFX mesh registered";
    } else if (ext == ".ztex") {
        itemType = AssetItemType::Texture;
        cookedOutPath = filePath;
        m_CurrentProgress.store(0.80f);
        statusMsg = "Pre-compiled ZeGFX texture registered";
    } else if (ext == ".zelyn" || ext == ".lua" || ext == ".cpp" || ext == ".cs") {
        itemType = AssetItemType::Script;
        m_CurrentProgress.store(0.80f);
        statusMsg = "Script module loaded";
    } else {
        itemType = AssetItemType::Unknown;
        m_CurrentProgress.store(0.80f);
        statusMsg = "Asset imported";
    }

    m_CurrentProgress.store(1.00f);
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_CurrentStatusText = "Finalizing asset registration...";
    }

    // Copy original or cooked file into project folder
    std::string targetPath = (fs::path(projectDir) / fileName).string();
    fs::copy_file(filePath, targetPath, fs::copy_options::overwrite_existing, ec);

    // Also copy sibling .bin buffers (essential for glTF assets)
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
            newNode.location[0] = 0.0f;
            newNode.location[1] = 0.0f;
            newNode.location[2] = 0.0f;

            // Add node to SceneGraph
            SceneGraph::Get().AddNode(newNode);

            // Auto-select the newly added SceneNode in EditorState
            EditorState::Get().selectedNodeName = newNode.name;
            EditorState::Get().selectedNodeType = SceneGraph::GetTypeName(newNode.type);
            EditorState::Get().selectedNodeNames = { newNode.name };

            Logger::Get().Info("[AssetCooker] Automatically instantiated SceneNode '" + newNode.name + "' with mesh " + result.outputCachePath);
        }
    }
}

void BackgroundAssetCooker::RenderCookingOverlay() {
    Update();

    bool isCooking = m_IsCooking.load();
    float progress = m_CurrentProgress.load();
    std::string taskName = GetCurrentTaskName();
    std::string statusText = GetCurrentStatusText();

    ImGuiIO& io = ImGui::GetIO();
    float padding = 16.0f;
    ImVec2 workPos = io.DisplaySize;
    ImVec2 windowPos = ImVec2(workPos.x - padding, workPos.y - padding);

    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.88f);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
                            ImGuiWindowFlags_AlwaysAutoResize |
                            ImGuiWindowFlags_NoSavedSettings |
                            ImGuiWindowFlags_NoFocusOnAppearing |
                            ImGuiWindowFlags_NoNav |
                            ImGuiWindowFlags_NoMove;

    if (isCooking || !m_Notifications.empty()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.24f, 0.52f, 0.88f, 0.60f));

        if (ImGui::Begin("##AssetCookingOverlay", nullptr, flags)) {
            if (isCooking) {
                ImGui::TextColored(ImVec4(0.28f, 0.68f, 1.00f, 1.00f), "[Asset Cooker]");
                ImGui::SameLine();
                ImGui::TextUnformatted(taskName.c_str());

                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.18f, 0.58f, 0.94f, 1.00f));
                char barBuf[64];
                snprintf(barBuf, sizeof(barBuf), "%.0f%%", progress * 100.0f);
                ImGui::ProgressBar(progress, ImVec2(220.0f, 14.0f), barBuf);
                ImGui::PopStyleColor();

                ImGui::TextDisabled("%s", statusText.c_str());
            }

            // Render Notifications below or instead of progress
            std::lock_guard<std::mutex> lock(m_Mutex);
            for (const auto& notif : m_Notifications) {
                if (isCooking) ImGui::Separator();
                if (notif.isSuccess) {
                    ImGui::TextColored(ImVec4(0.22f, 0.85f, 0.44f, 1.00f), "[SUCCESS] %s", notif.text.c_str());
                } else {
                    ImGui::TextColored(ImVec4(0.95f, 0.32f, 0.32f, 1.00f), "[ERROR] %s", notif.text.c_str());
                }
            }
        }
        ImGui::End();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);
    }
}

} // namespace EngineEditor
