#include "SplashScreen.h"
#include "stb_image.h"
#include "theme/Colors.h"
#include "theme/Metrics.h"

#include <iostream>
#include <cstring>
#include <imgui.h>

extern void WaitForGPU();

namespace EngineEditor {

SplashScreen& SplashScreen::Get() {
    static SplashScreen instance;
    return instance;
}

SplashScreen::SplashScreen() {
}

SplashScreen::~SplashScreen() {
    Shutdown();
}

void SplashScreen::Init(ID3D12Device* device, ID3D12CommandQueue* commandQueue, ID3D12DescriptorHeap* srvHeap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) {
    m_Device = device;
    m_CommandQueue = commandQueue;
    m_SRVCpuHandle = cpuHandle;
    m_SRVGpuHandle = gpuHandle;

    if (m_Device && m_CommandQueue) {
        LoadSplashTexture();
    }
}

void SplashScreen::Shutdown() {
    if (m_TextureResource) {
        WaitForGPU();
        m_TextureResource.Reset();
    }
    m_TextureLoaded = false;
}

void SplashScreen::LoadSplashTexture() {
    if (!m_Device || !m_CommandQueue) return;

    int width = 0, height = 0, channels = 0;
    unsigned char* pixels = stbi_load("assets/compilescreen/csli.jpg", &width, &height, &channels, 4);
    if (!pixels) {
        pixels = stbi_load("../assets/compilescreen/csli.jpg", &width, &height, &channels, 4);
    }
    if (!pixels) {
        pixels = stbi_load("y:/ZelynVMSL/ZeGFX/BluemanEngine/assets/compilescreen/csli.jpg", &width, &height, &channels, 4);
    }

    if (!pixels) {
        std::cerr << "[SplashScreen] Warning: Could not load splash screen image csli.jpg" << std::endl;
        return;
    }

    m_TextureWidth = (uint32_t)width;
    m_TextureHeight = (uint32_t)height;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = m_TextureWidth;
    texDesc.Height = m_TextureHeight;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    HRESULT hr = m_Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_TextureResource)
    );

    if (SUCCEEDED(hr)) {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
        UINT numRows = 0;
        UINT64 rowSizeInBytes = 0;
        UINT64 uploadBufferSize = 0;
        m_Device->GetCopyableFootprints(&texDesc, 0, 1, 0, &footprint, &numRows, &rowSizeInBytes, &uploadBufferSize);

        D3D12_HEAP_PROPERTIES uploadProps = {};
        uploadProps.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC uploadDesc = {};
        uploadDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        uploadDesc.Width = uploadBufferSize;
        uploadDesc.Height = 1;
        uploadDesc.DepthOrArraySize = 1;
        uploadDesc.MipLevels = 1;
        uploadDesc.Format = DXGI_FORMAT_UNKNOWN;
        uploadDesc.SampleDesc.Count = 1;
        uploadDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
        hr = m_Device->CreateCommittedResource(
            &uploadProps,
            D3D12_HEAP_FLAG_NONE,
            &uploadDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(&uploadBuffer)
        );

        if (SUCCEEDED(hr)) {
            BYTE* pData = nullptr;
            if (SUCCEEDED(uploadBuffer->Map(0, nullptr, (void**)&pData))) {
                BYTE* pDest = pData + footprint.Offset;
                for (UINT y = 0; y < m_TextureHeight; ++y) {
                    std::memcpy(pDest + y * footprint.Footprint.RowPitch, pixels + y * m_TextureWidth * 4, m_TextureWidth * 4);
                }
                uploadBuffer->Unmap(0, nullptr);

                Microsoft::WRL::ComPtr<ID3D12CommandAllocator> cmdAlloc;
                Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> cmdList;
                m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&cmdAlloc));
                m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAlloc.Get(), nullptr, IID_PPV_ARGS(&cmdList));

                D3D12_TEXTURE_COPY_LOCATION dstLoc = {};
                dstLoc.pResource = m_TextureResource.Get();
                dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
                dstLoc.SubresourceIndex = 0;

                D3D12_TEXTURE_COPY_LOCATION srcLoc = {};
                srcLoc.pResource = uploadBuffer.Get();
                srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
                srcLoc.PlacedFootprint = footprint;

                cmdList->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

                D3D12_RESOURCE_BARRIER barrier = {};
                barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
                barrier.Transition.pResource = m_TextureResource.Get();
                barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
                barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
                cmdList->ResourceBarrier(1, &barrier);

                cmdList->Close();
                ID3D12CommandList* ppCmdLists[] = { cmdList.Get() };
                m_CommandQueue->ExecuteCommandLists(1, ppCmdLists);
                WaitForGPU();

                D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
                srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
                srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = 1;

                m_Device->CreateShaderResourceView(m_TextureResource.Get(), &srvDesc, m_SRVCpuHandle);
                m_TextureLoaded = true;
            }
        }
    }

    stbi_image_free(pixels);
}

void RenderSplashScreenUI() {
    if (!SplashScreen::Get().IsActive()) return;

    float progress = SplashScreen::Get().GetProgress();
    const std::string& statusText = SplashScreen::Get().GetStatus();

    ImGuiIO& io = ImGui::GetIO();
    const auto& pal = Theme::GetPalette();

    float splashW = io.DisplaySize.x;
    float splashH = io.DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(splashW, splashH), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoDocking |
                             ImGuiWindowFlags_NoSavedSettings;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, pal.bgHeader);
    ImGui::PushStyleColor(ImGuiCol_Border, pal.borderSubtle);

    if (ImGui::Begin("##UnrealSplashScreen", nullptr, flags)) {
        uint64_t textureID = SplashScreen::Get().GetTextureID();
        float barH = 32.0f;
        float fontH = ImGui::GetFontSize();
        float textY = (splashH - barH) + ((barH - 3.0f) - fontH) * 0.5f - 1.0f;

        if (textureID != 0) {
            ImGui::Image((ImTextureID)textureID, ImVec2(splashW, splashH - barH));
        } else {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(ImGui::GetCursorScreenPos(), ImVec2(ImGui::GetCursorScreenPos().x + splashW, ImGui::GetCursorScreenPos().y + splashH - barH), ImGui::ColorConvertFloat4ToU32(pal.bgHeader));
        }

        // Minimal Progress Bar & Text Overlay vertically centered at the bottom
        ImGui::SetCursorPos(ImVec2(16.0f, textY));
        ImGui::PushStyleColor(ImGuiCol_Text, pal.textSecondary);
        ImGui::TextUnformatted(statusText.c_str());
        ImGui::PopStyleColor();

        ImGui::SetCursorPos(ImVec2(0.0f, splashH - 3.0f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, pal.accent);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, pal.bgBase);
        ImGui::ProgressBar(progress, ImVec2(splashW, 3.0f), "");
        ImGui::PopStyleColor(2);
    }
    ImGui::End();

    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

} // namespace EngineEditor
