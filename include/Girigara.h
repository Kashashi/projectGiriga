#ifndef GIRIGARA_ENGINE_GIRIGARA_H
#define GIRIGARA_ENGINE_GIRIGARA_H

#include <memory>
#include <string>

#include <Window/GlfwContext.h>
#include <Window/WindowManager.h>
#include <Event/EventBus.h>
#include <System/KeyboardSystem.h>
#include <System/ConsoleSystem.h>
#include <Resource/ResourceManager.h>
#include <Script/ScriptEngine.h>
#include <Scene/SceneManager.h>
#include <Renderer/QuadMesh.h>
#include <Shader/Shader.h>
#include <UI/UIConsole.h>
#include <UI/UIWindow.h>

class Girigara {
public:
    Girigara(int width = 800, int height = 600, const std::string& title = "ProjectGirigara");
    ~Girigara() = default;

    Girigara(const Girigara&) = delete;
    Girigara& operator=(const Girigara&) = delete;

    // 啟動主迴圈
    void run();

    // 取得各大核心子系統參照
    EventBus& getEventBus() { return m_eventBus; }
    WindowManager& getWindow() { return *m_window; }
    KeyboardSystem& getKeyboard() { return m_keyboardSystem; }
    ResourceManager& getResources() { return m_resources; }
    ScriptEngine& getScriptEngine() { return m_scriptEngine; }
    SceneManager& getSceneManager() { return *m_sceneManager; }
    Shader& getUIShader() { return *m_uiShader; }
    FontLoader& getDefaultFont() { return *m_defaultFont; }

    // 場景切換便利包裝
    template <typename T, typename... Args>
    void switchScene(Args&&... args) {
        m_sceneManager->switchScene<T>(std::forward<Args>(args)...);
    }

private:
    void initSubsystems();
    void setupUI();
    void setupShortcuts();

    // 宣告順序決定 RAII 初始化與銷毀順序
    GlfwContext m_glfw;
    EventBus m_eventBus;
    KeyboardSystem m_keyboardSystem;
    std::unique_ptr<WindowManager> m_window;
    ResourceManager m_resources;
    ScriptEngine m_scriptEngine;
    std::unique_ptr<SceneManager> m_sceneManager;

    // 渲染資源
    Shader* m_uiShader{nullptr};
    FontLoader* m_defaultFont{nullptr};
    std::unique_ptr<QuadMesh> m_quadMesh;

    // 全域 UI 元件指標
    UIConsole* m_inGameConsole{nullptr};
    UIWindow* m_globalConsoleWindow{nullptr};

    bool m_isRunning{false};
};

#endif // GIRIGARA_ENGINE_GIRIGARA_H
