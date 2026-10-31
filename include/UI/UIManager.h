#ifndef GIRIGARA_UI_UI_MANAGER_H
#define GIRIGARA_UI_UI_MANAGER_H

#include <UI/UIContainer.h>
#include <UI/UIConsole.h>
#include <UI/UIWindow.h>
#include <Resource/ResourceManager.h>
#include <Font/FontLoader.h>
#include <Event/EventBus.h>
#include <System/KeyboardSystem.h>
#include <System/ConsoleSystem.h>
#include <System/LoggerSystem.h>

class UIManager {
public:
    UIManager(UIContainer& rootCanvas,
              FontLoader& defaultFont,
              EventBus& eventBus,
              KeyboardSystem& keyboard,
              ConsoleSystem& console,
              LoggerSystem& logger,
              ResourceManager& resources,
              float screenWidth, float screenHeight);

    void toggleConsole();
    UIConsole* getConsole() { return m_console; }

private:
    UIContainer& m_rootCanvas;
    UIConsole* m_console{nullptr};
};

#endif // GIRIGARA_UI_UI_MANAGER_H
