#ifndef GIRIGARA_SCENE_SCENE_H
#define GIRIGARA_SCENE_SCENE_H

#include <Shader/Shader.h>
#include <Event/EventBus.h>
#include <Resource/ResourceManager.h>
#include <UI/UIContainer.h>

class SceneManager; // 前置宣告

class Scene {
public:
    Scene(SceneManager& manager, EventBus& bus, ResourceManager& resources)
    : m_manager(manager), m_bus(bus), m_resources(resources) {}
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    // 生命週期勾子
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update(float dt) { (void)dt; }
    virtual void render(const Shader& shader) {
        m_uiRoot.render(shader); // 預設繪製場景專屬的局部 UI
    }

    UIContainer& getUIRoot() { return m_uiRoot; }

protected:
    SceneManager& m_manager;
    EventBus& m_bus;
    ResourceManager& m_resources;
    UIContainer m_uiRoot; // 每個場景自帶一個局部 UI 根節點
};

#endif // GIRIGARA_SCENE_SCENE_H
