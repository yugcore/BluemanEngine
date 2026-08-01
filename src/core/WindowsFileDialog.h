#ifndef WINDOWS_FILE_DIALOG_H
#define WINDOWS_FILE_DIALOG_H

#include <string>
#include <vector>

namespace EngineEditor {

enum class FileDialogType {
    ImportAsset,
    OpenScene,
    SaveScene,
    OpenShader,
    OpenTexture,
    AllFiles
};

class WindowsFileDialog {
public:
    // Opens native Windows File Open Dialog (modal)
    static std::vector<std::string> OpenFileDialog(
        FileDialogType type = FileDialogType::ImportAsset,
        const std::string& title = "Import Assets into Blueman Engine",
        bool allowMultiSelect = true
    );

    // Opens native Windows Folder Selection Dialog
    static std::string OpenFolderDialog(const std::string& title = "Select Target Asset Folder");
};

} // namespace EngineEditor

#endif // WINDOWS_FILE_DIALOG_H
