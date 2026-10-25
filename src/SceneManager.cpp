#include <Scene/SceneManager.h>
#include <Event/Event.h>

SceneManager::SceneManager(EventBus& bus, ResourceManager& resources, LoggerSystem& logger)
: m_bus(bus), m_resources(resources), m_logger(logger),
m_globalUI(0.0f, 0.0f, 800.0f, 600.0f) {
    m_globalUI.setPassThrough(true);
    registerEventHandlers();
}

SceneManager::~SceneManager() {
    if (m_currentScene) {
        m_currentScene->onExit();
        m_currentScene.reset();
    }
}

void SceneManager::registerEventHandlers() {
    m_bus.subscribe<KeyEvent>([this](KeyEvent& e) {
        if (e.isHandled) return;
        if (m_globalUI.handleKey(e.key, e.scancode, e.action, e.mods)) {
            e.isHandled = true;
            return;
        }
        if (m_currentScene) {
            m_currentScene->getUIRoot().handleKey(e.key, e.scancode, e.action, e.mods);
        }
    });

    m_bus.subscribe<MouseMovedEvent>([this](MouseMovedEvent& e) {
        if (e.isHandled) return;
        m_globalUI.handleMouseMove(e.x, e.y);
        if (m_currentScene) {
            m_currentScene->getUIRoot().handleMouseMove(e.x, e.y);
        }
    });

    m_bus.subscribe<MouseButtonEvent>([this](MouseButtonEvent& e) {
        if (e.isHandled) return;
        if (!m_globalUI.handleMouseButton(e.button, e.action, e.mods, e.x, e.y)) {
            if (m_currentScene) {
                m_currentScene->getUIRoot().handleMouseButton(e.button, e.action, e.mods, e.x, e.y);
            }
        }
    });
}

void SceneManager::applyPendingScene() {
    if (m_pendingSceneFactory) {
        auto factory = std::move(m_pendingSceneFactory);
        m_pendingSceneFactory = nullptr;
        factory(); // 執行切換與 onExit/onEnter
    }
}

void SceneManager::update(float dt) {
    // ★ 在幀開頭執行安全的場景交接
    applyPendingScene();

    if (m_currentScene) {
        m_currentScene->update(dt);
    }
    m_globalUI.update(dt);
}

void SceneManager::render(const Shader& shader) {
    if (m_currentScene) {
        m_currentScene->render(shader);
    }
    m_globalUI.render(shader);
}
