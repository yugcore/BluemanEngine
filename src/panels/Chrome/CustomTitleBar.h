#ifndef CHROME_CUSTOM_TITLE_BAR_H
#define CHROME_CUSTOM_TITLE_BAR_H

struct GLFWwindow;

namespace EngineEditor {
    void SetCustomTitleBarWindow(GLFWwindow* window);
    void RenderCustomTitleBar();
    
    // Returns the total height of the unified title bar (title + menu row)
    float GetTitleBarTotalHeight();
}

#endif // CHROME_CUSTOM_TITLE_BAR_H