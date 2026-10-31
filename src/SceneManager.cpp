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
    // ★ 關鍵：視窗大小改變事件集中管理
    m_bus.subscribe<WindowResizeEvent>([this](WindowResizeEvent& e) {
        float newW = static_cast<float>(e.width);
        float newH = static_cast<float>(e.height);

        // 1. 更新全域 UI 畫布大小，並校正全域飄浮視窗
        m_globalUI.setSize(newW, newH);
        for (auto& child : m_globalUI.getChildren()) {
            if (auto* window = dynamic_cast<UIWindow*>(child.get())) {
                window->clampPosition(newW, newH);
            }
        }

        // 2. 轉發給當前場景，更新場景根容器大小，並校正場景專屬視窗
        if (m_currentScene) {
            m_currentScene->onWindowResize(newW, newH);
        }
    });

    // 原有的 KeyEvent, MouseMovedEvent, MouseButtonEvent 保持不變...[cite: 37]
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
