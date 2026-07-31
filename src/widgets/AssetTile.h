#ifndef WIDGET_ASSET_TILE_H
#define WIDGET_ASSET_TILE_H

#include <string>
#include <imgui.h>

namespace EngineEditor::Widgets {

bool RenderAssetTile(const char* id, const char* name, const char* typeName, const ImVec4& typeColor, bool isSelected, float width = 140.0f, float height = 70.0f);

} // namespace EngineEditor::Widgets

#endif // WIDGET_ASSET_TILE_H
