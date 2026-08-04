#include "SunOrbitController.h"

namespace EngineEditor {

SunOrbitController& SunOrbitController::Get() {
    static SunOrbitController instance;
    return instance;
}

} // namespace EngineEditor
