#ifndef CHROME_MENU_BAR_H
#define CHROME_MENU_BAR_H

namespace EngineEditor {
    // Renders standalone main menu bar (legacy, kept for compatibility)
    void RenderMenuBar();
    
    // Renders menu bar contents inline (used by unified title bar)
    void RenderMenuBarContents();
}

#endif // CHROME_MENU_BAR_H
