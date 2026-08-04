#ifndef SUN_ORBIT_CONTROLLER_H
#define SUN_ORBIT_CONTROLLER_H

#include <functional>
#include <vector>
#include <cmath>
#include <algorithm>
#include "core/ComponentRegistry.h"

namespace EngineEditor {

// Observer Callback (Phase 5)
// Signature: void(const float direction[3], float elevationDegrees)
using SunChangeListener = std::function<void(const float[3], float)>;

class SunOrbitController {
public:
    static SunOrbitController& Get();

    SunOrbitController() = default;

    // Time-of-day state & tunables (Phase 3)
    float timeOfDay = 12.0f;             // 0.0 to 24.0 hours (12.0 = noon overhead)
    float dayDurationSeconds = 120.0f;   // Seconds for a full 24h cycle
    float fixedArcYaw = 45.0f;           // Fixed orbit arc yaw angle in degrees
    bool isPaused = true;                // Pause time progression toggle (static by default)

    // Direct Time Setter (for UI scrubbing)
    void SetTimeOfDay(float hours) {
        timeOfDay = std::fmod(hours, 24.0f);
        if (timeOfDay < 0.0f) timeOfDay += 24.0f;
    }

    // Accessor for Sky/Atmosphere systems
    float GetSunElevationDegrees() const {
        // pitch: -90 = noon (+90° elevation), 0 = horizon (0° elevation), +90 = midnight (-90° elevation)
        float pitch = (timeOfDay / 24.0f) * 360.0f - 90.0f;
        return -pitch;
    }

    // Main Tick (Phase 3)
    void Tick(float deltaTime, DirectionalLightComponent* lightComp, TransformComponent* transformComp) {
        if (!isPaused && dayDurationSeconds > 0.0f) {
            timeOfDay += (deltaTime / dayDurationSeconds) * 24.0f;
            timeOfDay = std::fmod(timeOfDay, 24.0f);
            if (timeOfDay < 0.0f) timeOfDay += 24.0f;
        }

        float pitch = (timeOfDay / 24.0f) * 360.0f - 90.0f;
        float yaw = fixedArcYaw;
        float roll = 0.0f;

        if (transformComp) {
            transformComp->rotation[0] = pitch;
            transformComp->rotation[1] = yaw;
            transformComp->rotation[2] = roll;
        }

        float rotDeg[3] = { pitch, yaw, roll };
        float lightDir[3];
        DirectionalLightComponent::ComputeDirectionFromRotation(rotDeg, lightDir);

        if (lightComp) {
            lightComp->isDirty = true;
        }

        // Phase 5: Broadcast to registered Sky/Atmosphere observers
        float elevation = GetSunElevationDegrees();
        NotifySunChanged(lightDir, elevation);
    }

    // Observer Pattern Event Subscription (Phase 5)
    uint32_t RegisterSunChangeListener(SunChangeListener listener) {
        m_Listeners.push_back(listener);
        return static_cast<uint32_t>(m_Listeners.size() - 1);
    }

    void NotifySunChanged(const float direction[3], float elevationDegrees) {
        for (const auto& listener : m_Listeners) {
            if (listener) {
                listener(direction, elevationDegrees);
            }
        }
    }

private:
    std::vector<SunChangeListener> m_Listeners;
};

} // namespace EngineEditor

#endif // SUN_ORBIT_CONTROLLER_H
