#include <Girigara.h>
#include <Script/ScriptBindings.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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

    ScriptBindings::registerAll(m_scriptEngine.getRawState(), m_logger);
    m_console.setScriptEngine(&m_scriptEngine);

    m_sceneManager = std::make_unique<SceneManager>(m_eventBus, m_resources, m_logger);
    m_quadMesh = std::make_unique<QuadMesh>();
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
                                                       m_logger
    );
    m_inGameConsole->setName("InGameConsole");

    // 2. 建立 Lua 飄浮視窗
    auto titleLabel = std::make_unique<UIText>(0.0f, 0.0f, *m_defaultFont, U"全域主控台 (Lua 驅動)", 20);
    titleLabel->setColor(1.0f, 0.85f, 0.3f, 1.0f);

    m_globalConsoleWindow = globalUI.emplaceChild<UIWindow>(
        200.0f, 150.0f, 400.0f, 260.0f,
        std::move(titleLabel)
    );
    m_globalConsoleWindow->setCloseOnEsc(true);
    m_globalConsoleWindow->setVisible(false);
    m_globalConsoleWindow->setName("GlobalConsole");

    auto* statusText = m_globalConsoleWindow->emplaceContent<UIText>(
        20.0f, 60.0f, *m_defaultFont, U"全域疊加層運作正常", 24
    );
    statusText->setColor(0.2f, 0.9f, 0.4f, 1.0f);

    try {
        m_resources.loadScript("console_float", std::vector<std::string>{
            "./assets/scripts/console_float.lua",
            "../assets/scripts/console_float.lua"
        });
        m_scriptEngine.executeResource(m_resources.getScript("console_float"));
    } catch (const std::exception& e) {
        m_logger.warn(std::string("腳本載入略過: ") + e.what());
    }
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
