#include "ImportHeightmapModal.h"
#include "core/EditorState.h"
#include "core/WindowsFileDialog.h"
#include "engine/scene/SceneGraph.h"
#include "engine/core/Logger.h"
#include "render/ZeGFXAdapter.h"
#include "theme/Colors.h"

#include "heightmap_importer.h"

#include <imgui.h>
#include <string>
#include <filesystem>
#include <algorithm>
#include <iostream>

namespace EngineEditor {

static char s_FilePath[512] = "";
static char s_TerrainName[128] = "Imported_Terrain";
static zegfx::HeightmapImportSettings s_Settings;
static zegfx::HeightmapImportResult s_InspectResult;
static std::string s_ErrorMessage = "";
static bool s_FileInspected = false;

void RenderImportHeightmapModal() {
    auto& state = EditorState::Get();
    if (!state.showImportHeightmapModal) return;

    // Check if initial path was supplied from EditorState
    if (!state.activeHeightmapImportPath.empty()) {
        snprintf(s_FilePath, sizeof(s_FilePath), "%s", state.activeHeightmapImportPath.c_str());
        state.activeHeightmapImportPath = "";
        s_FileInspected = false;
        s_ErrorMessage = "";
    }

    // Auto-inspect file header if path is present and not yet inspected
    if (!s_FileInspected && s_FilePath[0] != '\0' && std::filesystem::exists(s_FilePath)) {
        s_InspectResult = zegfx::HeightmapImporter::InspectFile(s_FilePath);
        if (s_InspectResult.success) {
            if (s_InspectResult.sourceWidth > 0 && s_InspectResult.sourceHeight > 0) {
                s_Settings.targetWidth = s_InspectResult.sourceWidth;
                s_Settings.targetHeight = s_InspectResult.sourceHeight;
            }
            std::filesystem::path p(s_FilePath);
            snprintf(s_TerrainName, sizeof(s_TerrainName), "Terrain_%s", p.stem().string().c_str());
            s_ErrorMessage = "";
        } else {
            s_ErrorMessage = s_InspectResult.errorMessage;
        }
        s_FileInspected = true;
    }

    ImGui::OpenPopup("Import Heightmap into Terrain##Modal");

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(560.0f, 480.0f), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::BeginPopupModal("Import Heightmap into Terrain##Modal", nullptr, flags)) {
        const auto& pal = Theme::GetPalette();

        ImGui::TextColored(pal.accent, "Import Heightmap Image for Terrain Generation");
        ImGui::Separator();
        ImGui::Spacing();

        // --- File Selector ---
        ImGui::Text("Heightmap File:");
        ImGui::PushItemWidth(420.0f);
        if (ImGui::InputText("##HeightmapPath", s_FilePath, sizeof(s_FilePath))) {
            s_FileInspected = false;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("...##BrowseHeightmap", ImVec2(60.0f, 0.0f))) {
            auto files = WindowsFileDialog::OpenFileDialog(
                FileDialogType::ImportAsset,
                "Select Heightmap File (PNG/RAW/BMP/TGA/HDR)",
                false
            );
            if (!files.empty()) {
                snprintf(s_FilePath, sizeof(s_FilePath), "%s", files[0].c_str());
                s_FileInspected = false;
            }
        }

        // --- Source File Readout ---
        if (s_FileInspected && s_InspectResult.success) {
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.12f, 0.14f, 0.16f, 1.0f));
            ImGui::BeginChild("FileReadout", ImVec2(0.0f, 60.0f), true);
            ImGui::TextColored(ImVec4(0.45f, 0.85f, 0.45f, 1.0f), "[Detected Format]");
            ImGui::Text("Source Resolution: %d x %d pixels | Bit Depth: %d-bit | Channels: %d",
                        s_InspectResult.sourceWidth, s_InspectResult.sourceHeight,
                        s_InspectResult.bitDepth, s_InspectResult.channels);
            ImGui::EndChild();
            ImGui::PopStyleColor();
        } else if (!s_ErrorMessage.empty()) {
            ImGui::TextColored(ImVec4(0.95f, 0.35f, 0.35f, 1.0f), "Error: %s", s_ErrorMessage.c_str());
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextColored(pal.textPrimary, "Terrain Mesh Parameters");
        ImGui::Spacing();

        // --- Name ---
        ImGui::Text("Terrain Node Name:");
        ImGui::InputText("##TerrainNodeName", s_TerrainName, sizeof(s_TerrainName));

        // --- Target Grid Dimensions ---
        ImGui::Text("Target Grid Width & Height:");
        static int gridPresetIdx = 2; // 512
        const char* presets[] = { "64 x 64", "128 x 128", "256 x 256", "512 x 512", "1024 x 1024", "2048 x 2048", "Source Image Size", "Custom" };
        if (ImGui::Combo("Preset Resolution", &gridPresetIdx, presets, IM_ARRAYSIZE(presets))) {
            switch (gridPresetIdx) {
                case 0: s_Settings.targetWidth = s_Settings.targetHeight = 64; break;
                case 1: s_Settings.targetWidth = s_Settings.targetHeight = 128; break;
                case 2: s_Settings.targetWidth = s_Settings.targetHeight = 256; break;
                case 3: s_Settings.targetWidth = s_Settings.targetHeight = 512; break;
                case 4: s_Settings.targetWidth = s_Settings.targetHeight = 1024; break;
                case 5: s_Settings.targetWidth = s_Settings.targetHeight = 2048; break;
                case 6:
                    if (s_InspectResult.sourceWidth > 0 && s_InspectResult.sourceHeight > 0) {
                        s_Settings.targetWidth = s_InspectResult.sourceWidth;
                        s_Settings.targetHeight = s_InspectResult.sourceHeight;
                    }
                    break;
                default: break;
            }
        }

        ImGui::SliderInt("Grid Width", &s_Settings.targetWidth, 16, 4096);
        ImGui::SliderInt("Grid Height", &s_Settings.targetHeight, 16, 4096);

        // --- Physical Spacing & Height Scale ---
        ImGui::SliderFloat("Cell Size (m)", &s_Settings.cellSize, 0.25f, 10.0f, "%.2fm");
        ImGui::SliderFloat("Height Scale (m)", &s_Settings.heightScale, 1.0f, 300.0f, "%.1fm");

        float extentX = (s_Settings.targetWidth - 1) * s_Settings.cellSize;
        float extentZ = (s_Settings.targetHeight - 1) * s_Settings.cellSize;
        ImGui::TextDisabled("World Extents: %.1fm x %.1fm (%.2f km²)", extentX, extentZ, (extentX * extentZ) / 1000000.0f);

        // --- Processing Toggles ---
        ImGui::Checkbox("Resample Image (Bilinear Filtering)", &s_Settings.resample);
        ImGui::SameLine();
        ImGui::Checkbox("Invert Elevation", &s_Settings.invertHeight);
        ImGui::Checkbox("Normalize Elevation Range [0..1]", &s_Settings.normalize);

        // --- RAW file dimensions if file is .raw ---
        std::filesystem::path curPath(s_FilePath);
        std::string ext = curPath.extension().string();
        if (ext == ".raw" || ext == ".r16" || ext == ".r8" || ext == ".bin") {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.85f, 0.85f, 0.35f, 1.0f), "RAW Image Format Parameters:");
            ImGui::InputInt("RAW Width", &s_Settings.rawWidth);
            ImGui::InputInt("RAW Height", &s_Settings.rawHeight);
            ImGui::Checkbox("Big Endian (16-bit)", &s_Settings.rawBigEndian);
        }

        // --- Live Triangle Count Estimation ---
        uint64_t totalVerts = static_cast<uint64_t>(s_Settings.targetWidth) * s_Settings.targetHeight;
        uint64_t totalTris = (static_cast<uint64_t>(s_Settings.targetWidth) - 1) * (s_Settings.targetHeight - 1) * 2;
        ImGui::Spacing();
        ImGui::TextDisabled("Estimated Mesh: %llu vertices | %llu triangles", totalVerts, totalTris);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // --- Action Buttons ---
        if (ImGui::Button("Import & Generate 3D Terrain", ImVec2(220.0f, 32.0f))) {
            if (s_FilePath[0] == '\0' || !std::filesystem::exists(s_FilePath)) {
                s_ErrorMessage = "Please select a valid heightmap image file!";
            } else {
                std::ofstream dbg("heightmap_import_crash_debug.log", std::ios::app);
                if (dbg.is_open()) dbg << "[ImportHeightmapModal] Button clicked! s_FilePath=" << s_FilePath << " targetW=" << s_Settings.targetWidth << " targetH=" << s_Settings.targetHeight << std::endl;

                std::string outErr;
                std::string meshKey;
                try {
                    meshKey = ZeGFXAdapter::Get().CreateTerrainFromHeightmap(
                        s_TerrainName,
                        s_FilePath,
                        s_Settings,
                        outErr
                    );
                } catch (const std::exception& e) {
                    if (dbg.is_open()) dbg << "[ImportHeightmapModal] EXCEPTION in CreateTerrainFromHeightmap: " << e.what() << std::endl;
                    s_ErrorMessage = std::string("Exception: ") + e.what();
                } catch (...) {
                    if (dbg.is_open()) dbg << "[ImportHeightmapModal] UNKNOWN EXCEPTION in CreateTerrainFromHeightmap" << std::endl;
                    s_ErrorMessage = "Unknown Exception in CreateTerrainFromHeightmap";
                }

                if (!meshKey.empty()) {
                    if (dbg.is_open()) dbg << "[ImportHeightmapModal] meshKey=" << meshKey << ". Creating SceneNode..." << std::endl;
                    try {
                        // Create SceneGraph node
                        SceneNode terrainNode;
                        terrainNode.id = SceneGraph::Get().GenerateNodeId();
                        terrainNode.name = s_TerrainName;
                        terrainNode.type = SceneNodeType::Terrain;
                        terrainNode.meshPath = meshKey;
                        terrainNode.materialPath = "DefaultPBRMaterial";
                        terrainNode.location[0] = 0.0f;
                        terrainNode.location[1] = 0.0f;
                        terrainNode.location[2] = 0.0f;

                        SceneGraph::Get().AddNode(terrainNode);
                        EditorState::Get().SetSelection(terrainNode.name, "Terrain");

                        Logger::Get().Info("[HeightmapImport] Successfully generated 3D terrain: " + terrainNode.name);

                        state.showImportHeightmapModal = false;
                        s_FileInspected = false;
                        if (dbg.is_open()) dbg << "[ImportHeightmapModal] SUCCESS! Modal closed." << std::endl;
                        ImGui::CloseCurrentPopup();
                    } catch (const std::exception& e) {
                        if (dbg.is_open()) dbg << "[ImportHeightmapModal] EXCEPTION in AddNode: " << e.what() << std::endl;
                        s_ErrorMessage = std::string("Exception adding node: ") + e.what();
                    }
                } else {
                    if (dbg.is_open()) dbg << "[ImportHeightmapModal] CreateTerrainFromHeightmap failed: " << outErr << std::endl;
                    s_ErrorMessage = outErr;
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100.0f, 32.0f))) {
            state.showImportHeightmapModal = false;
            s_FileInspected = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

} // namespace EngineEditor
