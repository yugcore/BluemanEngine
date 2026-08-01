#ifndef PANELS_CODEBASE_CODE_EDITOR_PANEL_H
#define PANELS_CODEBASE_CODE_EDITOR_PANEL_H

#include <string>

namespace EngineEditor {

void RenderCodeEditorPanel(bool* pOpen = nullptr);
void OpenCodeDocument(const std::string& filename, const std::string& content = "");
void CloseCodeDocument(const std::string& filename);

} // namespace EngineEditor

#endif // PANELS_CODEBASE_CODE_EDITOR_PANEL_H
