#ifndef GIRIGARA_SCENE_SCENE_MANAGER_H
#define GIRIGARA_SCENE_SCENE_MANAGER_H

#include <Scene/Scene.h>
#include <Event/EventBus.h>
#include <Resource/ResourceManager.h>
#include <System/LoggerSystem.h>
#include <UI/UIContainer.h>
#include <memory>
#include <utility>
#include <functional>

class SceneManager {
public:
    SceneManager(EventBus& bus, ResourceManager& resources, LoggerSystem& logger);
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    // ★ 關鍵：switchScene 只排程切換，不立刻銷毀當前場景
    template <typename T, typename... Args>
    void switchScene(Args&&... args) {
        m_pendingSceneFactory = [this, ...capturedArgs = std::forward<Args>(args)]() mutable {
            if (m_currentScene) {
                m_currentScene->onExit();
            }
            m_currentScene = std::make_unique<T>(*this, m_bus, m_resources, m_logger, std::move(capturedArgs)...);
            m_currentScene->onEnter();
        };
    }

    void update(float dt);
    void render(const Shader& shader);

    UIContainer& getGlobalUI() { return m_globalUI; }
    Scene* getCurrentScene() const { return m_currentScene.get(); }

private:
    void applyPendingScene();
    void registerEventHandlers();

    EventBus& m_bus;
    ResourceManager& m_resources;
    LoggerSystem& m_logger;

    std::unique_ptr<Scene> m_currentScene{nullptr};
    std::function<void()> m_pendingSceneFactory{nullptr}; // ★ 待切換場景的工廠閉包
    UIContainer m_globalUI;
};

#endif // GIRIGARA_SCENE_SCENE_MANAGER_H
