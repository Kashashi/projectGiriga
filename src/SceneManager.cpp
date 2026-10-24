#include <Scene/SceneManager.h>
#include <Event/Event.h>

SceneManager::SceneManager(EventBus& bus, ResourceManager& resources)
: m_bus(bus), m_resources(resources) {
    registerEventHandlers();
}

SceneManager::~SceneManager() {
    if (m_currentScene) {
        m_currentScene->onExit();
        m_currentScene.reset();
    }
}

void SceneManager::registerEventHandlers() {
    // 滑鼠移動：Global UI 優先攔截
    m_bus.subscribe<MouseMovedEvent>([this](MouseMovedEvent& e) {
        if (m_globalUI.handleMouseMove(e.x, e.y)) {
            e.isHandled = true;
            return;
        }
        auto* sceneUI = getCurrentUIRoot();
        if (sceneUI && sceneUI->handleMouseMove(e.x, e.y)) {
            e.isHandled = true;
        }
    });

    // 滑鼠點擊：Global UI 優先攔截，若點在置頂視窗上，底層 Scene 絕不觸發
    m_bus.subscribe<MouseButtonEvent>([this](MouseButtonEvent& e) {
        if (m_globalUI.handleMouseButton(e.button, e.action, e.mods, e.x, e.y)) {
            e.isHandled = true;
            return;
        }
        auto* sceneUI = getCurrentUIRoot();
        if (sceneUI && sceneUI->handleMouseButton(e.button, e.action, e.mods, e.x, e.y)) {
            e.isHandled = true;
        }
    });

    // 鍵盤事件：Global UI 優先處理 (如 Global 視窗聚焦時按 ESC 關閉)
    m_bus.subscribe<KeyPressedEvent>([this](KeyPressedEvent& e) {
        if (m_globalUI.handleKey(e.key, e.scancode, e.action, e.mods)) {
            e.isHandled = true;
            return;
        }
        auto* sceneUI = getCurrentUIRoot();
        if (sceneUI && sceneUI->handleKey(e.key, e.scancode, e.action, e.mods)) {
            e.isHandled = true;
        }
    });
}

void SceneManager::applyPendingScene() {
    if (m_pendingSceneFactory) {
        if (m_currentScene) {
            m_currentScene->onExit();
            m_currentScene.reset(); // 當前場景與局部 UI 釋放，m_globalUI 完好如初
        }
        m_currentScene = m_pendingSceneFactory();
        m_pendingSceneFactory = nullptr;
        if (m_currentScene) {
            m_currentScene->onEnter();
        }
    }
}

void SceneManager::update(float dt) {
    applyPendingScene();

    // 更新場景邏輯
    if (m_currentScene) {
        m_currentScene->update(dt);
    }

    // 更新全域 UI (若有 Lua 腳本動畫或計數更新)
    m_globalUI.update(dt);
}

void SceneManager::render(const Shader& shader) {
    // 1. 先畫當前場景與場景局部 UI
    if (m_currentScene) {
        m_currentScene->render(shader);
    }

    // 2. 最後畫全域 UI，確保永久覆蓋在最上層
    m_globalUI.render(shader);
}
