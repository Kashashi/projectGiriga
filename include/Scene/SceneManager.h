#ifndef GIRIGARA_SCENE_SCENE_MANAGER_H
#define GIRIGARA_SCENE_SCENE_MANAGER_H

#include <Scene/Scene.h>
#include <UI/UIContainer.h>
#include <memory>
#include <functional>

class SceneManager {
public:
    SceneManager(EventBus& bus, ResourceManager& resources);
    ~SceneManager();

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    template <typename T, typename... Args>
    void switchScene(Args&&... args) {
        m_pendingSceneFactory = [this, ...args = std::forward<Args>(args)]() mutable {
            return std::make_unique<T>(*this, m_bus, m_resources, std::forward<Args>(args)...);
        };
    }

    void update(float dt);
    void render(const Shader& shader);

    // 取得當前場景的局部 UI (切換場景會自動銷毀)
    UIContainer* getCurrentUIRoot() {
        return m_currentScene ? &m_currentScene->getUIRoot() : nullptr;
    }

    // 取得永遠置頂、不隨場景切換銷毀的全域 UI 容器
    UIContainer& getGlobalUI() { return m_globalUI; }

private:
    EventBus& m_bus;
    ResourceManager& m_resources;
    std::unique_ptr<Scene> m_currentScene = nullptr;
    std::function<std::unique_ptr<Scene>()> m_pendingSceneFactory = nullptr;

    // 全域頂層 UI 容器 (生命週期與 SceneManager 相同)
    UIContainer m_globalUI;

    void applyPendingScene();
    void registerEventHandlers();
};

#endif // GIRIGARA_SCENE_SCENE_MANAGER_H
