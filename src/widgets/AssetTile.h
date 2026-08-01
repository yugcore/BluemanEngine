#ifndef WIDGET_ASSET_TILE_H
#define WIDGET_ASSET_TILE_H

#include <string>
#include <imgui.h>
#include "core/AssetRegistry.h"

namespace EngineEditor::Widgets {

bool RenderAssetTile(const char* id, const char* name, AssetItemType itemType, const char* typeName, const ImVec4& typeColor, bool isSelected, float width = 120.0f, float height = 128.0f);

} // namespace EngineEditor::Widgets

#endif // WIDGET_ASSET_TILE_H
