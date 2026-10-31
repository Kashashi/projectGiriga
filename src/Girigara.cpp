#include <Girigara.h>
#include <Script/ScriptBindings.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

namespace fs = std::filesystem;

Girigara::Girigara(int width, int height, const std::string& title)
: m_logger(65535),
m_glfw(),
m_eventBus(),
m_keyboardSystem(m_eventBus),
m_window(std::make_unique<WindowManager>(width, height, title, m_eventBus, m_logger)),
m_resources(m_logger),
m_scriptEngine(),
m_console(m_logger) {

    m_logger.info("引擎初始化開始...");

    GlfwContext::initGLAD();

    initSubsystems();
    setupUI();
    setupShortcuts();

    m_logger.info("引擎初始化完成。");
}

void Girigara::initSubsystems() {
    m_uiShader = &m_resources.loadShader("ui", "./assets/shaders/ui.vert", "./assets/shaders/ui.frag");
    m_window->setShader(m_uiShader);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window->getWindow(), &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    m_window->updateProjection(fbWidth, fbHeight);

    m_defaultFont = &m_resources.loadFont("default", {
        "./assets/font/NaikaiFont-Regular.ttf",
        "/usr/share/fonts/google-noto-cjk-fonts/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
    }, 48);

    // 1. 註冊 Lua C++ 綁定與連接 Console
    ScriptBindings::registerAll(m_scriptEngine.getRawState(), m_logger);
    m_console.setScriptEngine(&m_scriptEngine);

    // 2. ★ 載入 ./assets/sysScripts/ 裡的所有 Lua 系統腳本
    loadSystemScripts("./assets/sysScripts");

    m_sceneManager = std::make_unique<SceneManager>(m_eventBus, m_resources, m_logger);
    m_quadMesh = std::make_unique<QuadMesh>();
}

void Girigara::loadSystemScripts(const std::string& directoryPath) {
    fs::path dir(directoryPath);

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        m_logger.warn("[Script] 找不到系統腳本資料夾: " + directoryPath);
        return;
    }

    m_logger.info("[Script] 開始載入系統腳本目錄: " + directoryPath);

    // 遞迴掃描目錄下所有檔案
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".lua") {
            std::string filePath = entry.path().string();
            // 使用去除副檔名的檔名作為資源名稱，例如 "debug.lua" -> "debug"
            std::string scriptName = entry.path().stem().string();

            try {
                // 由 ResourceManager 載入並快取[cite: 14]
                m_resources.loadScript(scriptName, filePath);
                const auto& scriptRes = m_resources.getScript(scriptName);

                // 立即由 ScriptEngine 執行一次，將全域函式註冊進虛擬機
                if (m_scriptEngine.executeResource(scriptRes)) {
                    m_logger.info("[Script] 已註冊系統腳本: " + scriptName + " (" + filePath + ")");
                } else {
                    m_logger.error("[Script] 執行腳本出錯: " + filePath);
                }
            } catch (const std::exception& e) {
                m_logger.error(std::string("[Script] 載入例外: ") + e.what());
            }
        }
    }
}

void Girigara::setupUI() {
    auto& globalUI = m_sceneManager->getGlobalUI();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window->getWindow(), &fbWidth, &fbHeight);

    // 1. 建立 In-Game Console
    m_inGameConsole = globalUI.emplaceChild<UIConsole>(
        0.0f, 0.0f,
        static_cast<float>(fbWidth),
                                                       static_cast<float>(fbHeight),
                                                       *m_defaultFont,
                                                       m_eventBus,
                                                       m_console,
                                                       m_logger,
                                                       m_resources
    );
    m_inGameConsole->setName("InGameConsole");
}

void Girigara::setupShortcuts() {
    auto& globalUI = m_sceneManager->getGlobalUI();

    // ` (Grave Accent / Tilde): 切換 In-Game Console
    m_keyboardSystem.registerShortcut(
        GLFW_KEY_GRAVE_ACCENT,
        0,
        [this, &globalUI]() {
            if (!m_inGameConsole) return;
            bool nextVisible = !m_inGameConsole->isVisible();
            m_inGameConsole->setVisible(nextVisible);
            if (nextVisible) {
                globalUI.setFocusedElement(m_inGameConsole);
            }
        }
    );

    // Ctrl + Shift + U: 超級全局快捷鍵 (在 Console 開啟時依然有效)
    m_keyboardSystem.registerShortcut(
        GLFW_KEY_U,
        GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
        [this, &globalUI]() {
            if (!m_globalConsoleWindow) return;
            bool nextVisible = !m_globalConsoleWindow->isVisible();
            m_globalConsoleWindow->setVisible(nextVisible);
            if (nextVisible) {
                globalUI.setFocusedElement(m_globalConsoleWindow);
            }
        }
    );
}

void Girigara::run() {
    m_isRunning = true;
    float lastTime = static_cast<float>(glfwGetTime());

    while (m_isRunning && !m_window->shouldClose()) {
        float currentTime = static_cast<float>(glfwGetTime());
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        m_window->pollEvents();
        m_sceneManager->update(dt);

        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        m_uiShader->use();
        glBindVertexArray(m_quadMesh->getVAO());
        m_sceneManager->render(*m_uiShader);

        m_window->swapBuffers();
    }
}
