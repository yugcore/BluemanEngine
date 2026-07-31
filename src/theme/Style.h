#ifndef THEME_STYLE_H
#define THEME_STYLE_H

#include "Colors.h"
#include "Metrics.h"

namespace EngineEditor::Theme {

void ApplyMasterStyle(const Palette& palette = Palette(), const Metrics& metrics = Metrics());

} // namespace EngineEditor::Theme

#endif // THEME_STYLE_H
