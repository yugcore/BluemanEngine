#ifndef WIDGET_PROPERTY_ROW_H
#define WIDGET_PROPERTY_ROW_H

#include <imgui.h>

namespace EngineEditor::Widgets {

bool RenderVector3PropertyRow(const char* label, float values[3], float resetValue = 0.0f, bool* lockAspect = nullptr);

} // namespace EngineEditor::Widgets

#endif // WIDGET_PROPERTY_ROW_H
