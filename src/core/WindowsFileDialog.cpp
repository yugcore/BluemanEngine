#include "WindowsFileDialog.h"
#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <iostream>
#include <vector>
#include <string>

namespace EngineEditor {

std::vector<std::string> WindowsFileDialog::OpenFileDialog(FileDialogType type, const std::string& title, bool allowMultiSelect) {
    std::vector<std::string> results;

    char filenameBuffer[65536] = { 0 };

    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = GetActiveWindow();
    ofn.lpstrFile = filenameBuffer;
    ofn.nMaxFile = sizeof(filenameBuffer);
    ofn.lpstrTitle = title.c_str();

    const char* filter = nullptr;
    switch (type) {
    case FileDialogType::ImportAsset:
        filter = "All Supported Assets (*.fbx;*.gltf;*.glb;*.obj;*.png;*.jpg;*.jpeg;*.tga;*.dds;*.hdr;*.exr;*.zmesh;*.ztex;*.zasset)\0"
                 "*.fbx;*.gltf;*.glb;*.obj;*.png;*.jpg;*.jpeg;*.tga;*.dds;*.hdr;*.exr;*.zmesh;*.ztex;*.zasset\0"
                 "3D Meshes & Geometry (*.fbx;*.gltf;*.glb;*.obj;*.zmesh)\0"
                 "*.fbx;*.gltf;*.glb;*.obj;*.zmesh\0"
                 "Textures & Images (*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.dds;*.hdr;*.exr;*.ztex)\0"
                 "*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.dds;*.hdr;*.exr;*.ztex\0"
                 "Compiled ZeGFX Binary Cache (*.zmesh;*.ztex;*.zasset;*.zeshader)\0"
                 "*.zmesh;*.ztex;*.zasset;*.zeshader\0"
                 "Zelyn Scripts & Code (*.zelyn;*.lua;*.cpp;*.cs)\0"
                 "*.zelyn;*.lua;*.cpp;*.cs\0"
                 "All Files (*.*)\0"
                 "*.*\0\0";
        break;
    case FileDialogType::OpenScene:
        filter = "Blueman Scene Files (*.zscene;*.json)\0*.zscene;*.json\0All Files (*.*)\0*.*\0\0";
        break;
    case FileDialogType::OpenShader:
        filter = "HLSL Shaders & ZeGFX Variants (*.hlsl;*.zeshader;*.zeshaderlib)\0*.hlsl;*.zeshader;*.zeshaderlib\0All Files (*.*)\0*.*\0\0";
        break;
    case FileDialogType::OpenTexture:
        filter = "Textures (*.png;*.jpg;*.jpeg;*.tga;*.dds;*.ztex)\0*.png;*.jpg;*.jpeg;*.tga;*.dds;*.ztex\0All Files (*.*)\0*.*\0\0";
        break;
    default:
        filter = "All Files (*.*)\0*.*\0\0";
        break;
    }

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;
    if (allowMultiSelect) {
        ofn.Flags |= OFN_ALLOWMULTISELECT;
    }

    if (GetOpenFileNameA(&ofn)) {
        // Handle multi-selection or single-selection
        char* p = filenameBuffer;
        std::string dirOrFile = p;
        p += dirOrFile.length() + 1;

        if (*p == '\0') {
            // Single file selected
            results.push_back(dirOrFile);
        } else {
            // Multiple files selected; dirOrFile is the directory path
            std::string dir = dirOrFile;
            if (!dir.empty() && dir.back() != '\\' && dir.back() != '/') {
                dir += "\\";
            }
            while (*p != '\0') {
                std::string file = p;
                results.push_back(dir + file);
                p += file.length() + 1;
            }
        }
    }

    return results;
}

std::string WindowsFileDialog::OpenFolderDialog(const std::string& title) {
    std::string folderPath = "";
    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = GetActiveWindow();
    bi.lpszTitle = title.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != 0) {
        char path[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, path)) {
            folderPath = path;
        }
        IMalloc* imalloc = 0;
        if (SUCCEEDED(SHGetMalloc(&imalloc))) {
            imalloc->Free(pidl);
            imalloc->Release();
        }
    }
    return folderPath;
}

} // namespace EngineEditor
