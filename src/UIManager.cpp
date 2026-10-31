#include <UI/UIManager.h>

UIManager::UIManager(UIContainer& rootCanvas,
                     FontLoader& defaultFont,
                     EventBus& eventBus,
                     KeyboardSystem& keyboard,
                     ConsoleSystem& console,
                     LoggerSystem& logger,
                     ResourceManager& resources,
                     float screenWidth, float screenHeight)
: m_rootCanvas(rootCanvas) {

    // 建立 In-Game Console
    m_console = m_rootCanvas.emplaceChild<UIConsole>(
        0.0f, 0.0f, screenWidth, screenHeight,
        defaultFont, eventBus, console, logger, resources
    );
    m_console->setName("InGameConsole");

    // 註冊切換主控台快捷鍵 (Grave Accent / Tilde)
    keyboard.registerShortcut(
        GLFW_KEY_GRAVE_ACCENT, 0,
        [this]() { toggleConsole(); }
    );
}

void UIManager::toggleConsole() {
    if (!m_console) return;
    bool next = !m_console->isVisible();
    m_console->setVisible(next);
    if (next) {
        m_rootCanvas.setFocusedElement(m_console);
    }
}
