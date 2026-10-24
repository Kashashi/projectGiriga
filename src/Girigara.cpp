#include <Girigara.h>
#include <Script/ScriptBindings.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

Girigara::Girigara(int width, int height, const std::string& title)
: m_glfw(),
m_eventBus(),
m_keyboardSystem(m_eventBus),
m_window(std::make_unique<WindowManager>(width, height, title, m_eventBus)) {

    // 1. 初始化 OpenGL 函式庫
    GlfwContext::initGLAD();

    // 2. 核心資源與子系統初始化
    initSubsystems();

    // 3. 建置預設全域 UI 與 In-Game Console
    setupUI();

    // 4. 註冊系統全域快捷鍵
    setupShortcuts();
}

void Girigara::initSubsystems() {
    // 載入預設 UI Shader
    m_uiShader = &m_resources.loadShader("ui", "./assets/shaders/ui.vert", "./assets/shaders/ui.frag");
    m_window->setShader(m_uiShader);

    // 設定 Viewport 與正交矩陣
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window->getWindow(), &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);
    m_window->updateProjection(fbWidth, fbHeight);

    // 載入多路徑字型
    m_defaultFont = &m_resources.loadFont("default", {
        "./assets/font/NaikaiFont-Regular.ttf",
        "/usr/share/fonts/google-noto-cjk-fonts/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
    }, 48);

    // 註冊 Lua API 並連結控制台
    ScriptBindings::registerAll(m_scriptEngine.getRawState());
    ConsoleSystem::getInstance().setScriptEngine(&m_scriptEngine);

    // 初始化場景管理器
    m_sceneManager = std::make_unique<SceneManager>(m_eventBus, m_resources);
    m_quadMesh = std::make_unique<QuadMesh>();
}

void Girigara::setupUI() {
    auto& globalUI = m_sceneManager->getGlobalUI();

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(m_window->getWindow(), &fbWidth, &fbHeight);

    // 建立 In-Game Console（初始即填滿當前視窗，高設為 fbHeight）
    m_inGameConsole = globalUI.emplaceChild<UIConsole>(
        0.0f, 0.0f,
        static_cast<float>(fbWidth),
        static_cast<float>(fbHeight),
        *m_defaultFont,
        m_eventBus
    );
    m_inGameConsole->setName("InGameConsole");

    // 綁定視窗尺寸變動事件 (或直接在 FramebufferSize 派發時同步更新)
    m_eventBus.subscribe<WindowFocusEvent>([this](WindowFocusEvent&) {
        int w, h;
        glfwGetFramebufferSize(m_window->getWindow(), &w, &h);
        if (m_inGameConsole) {
            m_inGameConsole->onResize(static_cast<float>(w), static_cast<float>(h));
        }
    });

    // 建立 Lua 動態示範視窗
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

    // 建立 In-Game Console（寬 800，高 450）
    m_inGameConsole = globalUI.emplaceChild<UIConsole>(0.0f, 0.0f, 800.0f, 450.0f, *m_defaultFont, m_eventBus);
    m_inGameConsole->setName("InGameConsole");

    // 嘗試執行全域預設 Lua 腳本
    try {
        m_resources.loadScript("console_float", std::vector<std::string>{
            "./assets/scripts/console_float.lua",
            "../assets/scripts/console_float.lua"
        });
        m_scriptEngine.executeResource(m_resources.getScript("console_float"));
    } catch (const std::exception& e) {
        ConsoleSystem::getInstance().log(std::string("[Warning] ") + e.what());
    }
}

void Girigara::setupShortcuts() {
    auto& globalUI = m_sceneManager->getGlobalUI();

    // ` (Grave Accent / Tilde): 切換 In-Game Console
    m_keyboardSystem.registerShortcut(
        GLFW_KEY_GRAVE_ACCENT,
        0,
        [this, &globalUI]() {
            bool nextVisible = !m_inGameConsole->isVisible();
            m_inGameConsole->setVisible(nextVisible);
            if (nextVisible) {
                globalUI.setFocusedElement(m_inGameConsole);
            }
        }
    );

    // Ctrl + Shift + U: 切換 Lua 飄浮視窗
    m_keyboardSystem.registerShortcut(
        GLFW_KEY_U,
        GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
        [this, &globalUI]() {
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

        // 1. 事件輪詢
        m_window->pollEvents();

        // 2. 邏輯更新 (場景 + UI)
        m_sceneManager->update(dt);

        // 3. 畫面清除
        glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 4. 繪製場景與 UI
        m_uiShader->use();
        glBindVertexArray(m_quadMesh->getVAO());
        m_sceneManager->render(*m_uiShader);

        // 5. 緩衝區交換
        m_window->swapBuffers();
    }
}
