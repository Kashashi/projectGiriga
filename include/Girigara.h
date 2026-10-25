#ifndef GIRIGARA_ENGINE_GIRIGARA_H
#define GIRIGARA_ENGINE_GIRIGARA_H

#include <memory>
#include <string>

#include <System/LoggerSystem.h>
#include <System/ConsoleSystem.h>
#include <Window/GlfwContext.h>
#include <Window/WindowManager.h>
#include <Event/EventBus.h>
#include <System/KeyboardSystem.h>
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

    void run();

    LoggerSystem& getLogger() { return m_logger; }
    ConsoleSystem& getConsole() { return m_console; }
    EventBus& getEventBus() { return m_eventBus; }
    WindowManager& getWindow() { return *m_window; }
    ResourceManager& getResources() { return m_resources; }
    ScriptEngine& getScriptEngine() { return m_scriptEngine; }
    SceneManager& getSceneManager() { return *m_sceneManager; }

    template <typename T, typename... Args>
    void switchScene(Args&&... args) {
        m_sceneManager->switchScene<T>(std::forward<Args>(args)...);
    }

private:
    void initSubsystems();
    void setupUI();
    void setupShortcuts();

    // RAII 宣告順序：LoggerSystem 位於最頂部
    LoggerSystem m_logger;
    GlfwContext m_glfw;
    EventBus m_eventBus;
    KeyboardSystem m_keyboardSystem;
    std::unique_ptr<WindowManager> m_window;
    ResourceManager m_resources;
    ScriptEngine m_scriptEngine;
    ConsoleSystem m_console;
    std::unique_ptr<SceneManager> m_sceneManager;

    Shader* m_uiShader{nullptr};
    FontLoader* m_defaultFont{nullptr};
    std::unique_ptr<QuadMesh> m_quadMesh;

    UIConsole* m_inGameConsole{nullptr};
    UIWindow* m_globalConsoleWindow{nullptr};

    bool m_isRunning{false};
};

#endif // GIRIGARA_ENGINE_GIRIGARA_H
