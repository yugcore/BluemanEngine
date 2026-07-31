#ifndef WIDGET_SEARCH_BAR_H
#define WIDGET_SEARCH_BAR_H

#include <imgui.h>

namespace EngineEditor::Widgets {

bool RenderSearchBar(const char* label, char* buffer, size_t bufferSize, const char* hint = "Search...", float width = -1.0f);

} // namespace EngineEditor::Widgets

#endif // WIDGET_SEARCH_BAR_H
