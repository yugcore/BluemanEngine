#ifndef THEME_H
#define THEME_H

#include "Colors.h"
#include "Metrics.h"
#include "Fonts.h"
#include "Style.h"

namespace EngineEditor {
    inline void ApplyTheme() {
        Theme::ApplyMasterStyle();
    }
}

#endif // THEME_H
