#ifndef GIRIGARA_SCENE_SCENE_H
#define GIRIGARA_SCENE_SCENE_H

#include <Shader/Shader.h>
#include <Event/EventBus.h>
#include <Resource/ResourceManager.h>
#include <UI/UIContainer.h>
#include <UI/UIWindow.h>
#include <GLFW/glfw3.h>

class SceneManager;
class LoggerSystem;

class Scene {
public:
    Scene(SceneManager& manager, EventBus& bus, ResourceManager& resources, LoggerSystem& logger)
    : m_manager(manager), m_bus(bus), m_resources(resources), m_logger(logger) {
        // ★ 取得當前 GLFW 視窗真實解析度，鋪滿整個根畫布
        int fbW = 800, fbH = 600;
        GLFWwindow* win = glfwGetCurrentContext();
        if (win) {
            glfwGetFramebufferSize(win, &fbW, &fbH);
        }
        m_uiRoot.setPosition(0.0f, 0.0f);
        m_uiRoot.setSize(static_cast<float>(fbW), static_cast<float>(fbH));
        m_uiRoot.setPassThrough(true); // 根容器允許未點擊處穿透給遊戲世界[cite: 24]
    }

    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) noexcept = default;
    Scene& operator=(Scene&&) noexcept = default;

    // 生命週期勾子
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    // ★ 預設將幀時間傳給場景專屬的 Root UI 容器，驅動子元件與排版器更新
    virtual void update(float dt) {
        m_uiRoot.update(dt);
    }
    virtual void render(const Shader& shader) {
        m_uiRoot.render(shader);
    }

    // ★ 由 SceneManager 在視窗縮放時呼叫
    virtual void onWindowResize(float newWidth, float newHeight) {
        m_uiRoot.setSize(newWidth, newHeight);

        // 遞迴/遍歷將所有超出畫面的子 UIWindow 推回畫面合法範圍內
        for (auto& child : m_uiRoot.getChildren()) {
            if (auto* window = dynamic_cast<UIWindow*>(child.get())) {
                window->clampPosition(newWidth, newHeight);
            }
        }
    }

    UIContainer& getUIRoot() { return m_uiRoot; }

protected:
    SceneManager& m_manager;
    EventBus& m_bus;
    ResourceManager& m_resources;
    UIContainer m_uiRoot; // ★ 包含整個場景畫面的根節點
    LoggerSystem& m_logger;
};

#endif // GIRIGARA_SCENE_SCENE_H
