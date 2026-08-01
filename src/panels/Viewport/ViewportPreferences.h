#ifndef VIEWPORT_PREFERENCES_H
#define VIEWPORT_PREFERENCES_H

#include <imgui.h>

namespace EngineEditor::Panels {

enum class UnitSystem {
    MetricMeters,
    MetricCentimeters,
    ImperialFeet
};

struct ViewportPreferences {
    static ViewportPreferences& Get() {
        static ViewportPreferences instance;
        return instance;
    }

    UnitSystem unitSystem = UnitSystem::MetricMeters;
    
    ImU32 gridColor = IM_COL32(160, 175, 190, 140);
    ImU32 majorGridColor = IM_COL32(110, 130, 150, 180);
    ImU32 xAxisColor = IM_COL32(235, 65, 65, 230);
    ImU32 zAxisColor = IM_COL32(65, 135, 245, 230);
    ImU32 yAxisColor = IM_COL32(65, 220, 90, 230);

    ImU32 selectionOutlineColor = IM_COL32(255, 215, 0, 240);
    ImU32 selectionGlowColor = IM_COL32(255, 235, 100, 255);
    float outlineThickness = 2.0f;

    bool showSafeFrame = false;
    bool showAspectGuides = false;
    bool showRuleOfThirds = false;
    bool showCenterCrosshair = false;
    bool showStatsHUD = true;
    bool showFrustums = true;
    bool xraySelection = false;
    bool cameraCollision = true;

    int activeAspectIndex = 0; // 0: 16:9, 1: 21:9, 2: 4:3, 3: 1:1
};

} // namespace EngineEditor::Panels

#endif // VIEWPORT_PREFERENCES_H
