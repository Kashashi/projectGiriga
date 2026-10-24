#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>
#include <vector>

#include <Window/GlfwContext.h>
#include <Window/WindowManager.h>
#include <Event/EventBus.h>
#include <System/KeyboardSystem.h>
#include <Shader/Shader.h>
#include <Resource/ResourceManager.h>
#include <Renderer/QuadMesh.h>
#include <Scene/SceneManager.h>
#include <Scene/MainMenuScene.h>
#include <Script/ScriptEngine.h>
#include <Script/ScriptBindings.h>
#include <UI/UIText.h>
#include <UI/UIWindow.h>

int main() {
	// 1. RAII 初始化 GLFW (解構時自動呼叫 glfwTerminate)
	GlfwContext glfw;

	EventBus eventBus;
	KeyboardSystem keyboardSystem(eventBus);
	auto appWindow = std::make_unique<WindowManager>(800, 600, "Scene Management Architecture", eventBus);

	// 2. 初始化 GLAD 與全域 OpenGL 狀態
	GlfwContext::initGLAD();

	// 3. 資源載入
	ResourceManager resources;
	Shader& uiShader = resources.loadShader("ui", "./assets/shaders/ui.vert", "./assets/shaders/ui.frag");
	FontLoader& font = resources.loadFont("default", {
		"./assets/font/NaikaiFont-Regular.ttf",
		"/usr/share/fonts/google-noto-cjk-fonts/NotoSansCJK-Regular.ttc",
		"/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
	}, 48);

	appWindow->setShader(&uiShader);

	int fbWidth, fbHeight;
	glfwGetFramebufferSize(appWindow->getWindow(), &fbWidth, &fbHeight);
	glViewport(0, 0, fbWidth, fbHeight);
	appWindow->updateProjection(fbWidth, fbHeight);

	// 4. 初始化 Lua 腳本引擎並註冊 C-API
	ScriptEngine scriptEngine;
	ScriptBindings::registerAll(scriptEngine.getRawState());

	// 5. 初始化場景管理器與全域頂層 UI
	SceneManager sceneManager(eventBus, resources);
	auto& globalUI = sceneManager.getGlobalUI();

	auto titleLabel = std::make_unique<UIText>(0.0f, 0.0f, font, U"全域主控台 (Lua 驅動)", 20);
	titleLabel->setColor(1.0f, 0.85f, 0.3f, 1.0f);

	auto* globalConsole = globalUI.emplaceChild<UIWindow>(
		200.0f, 150.0f, 400.0f, 260.0f,
		std::move(titleLabel)
	);
	globalConsole->setCloseOnEsc(true);
	globalConsole->setVisible(false);
	globalConsole->setName("GlobalConsole");

	auto* statusText = globalConsole->emplaceContent<UIText>(
		20.0f, 60.0f, font, U"全域疊加層運作正常", 24
	);
	statusText->setColor(0.2f, 0.9f, 0.4f, 1.0f);

	// 6. 載入並執行 Lua 腳本
	resources.loadScript("console_float", std::vector<std::string>{
		"./assets/scripts/console_float.lua",
		"../assets/scripts/console_float.lua"
	});
	scriptEngine.executeResource(resources.getScript("console_float"));

	// 7. 註冊全域快捷鍵：Ctrl + Shift + U 切換視窗
	keyboardSystem.registerShortcut(
		GLFW_KEY_U,
		GLFW_MOD_CONTROL | GLFW_MOD_SHIFT,
		[globalConsole, &globalUI]() {
			bool nextVisible = !globalConsole->isVisible();
			globalConsole->setVisible(nextVisible);
			if (nextVisible) {
				globalUI.setFocusedElement(globalConsole);
			}
		}
	);

	// 8. 啟動第一個場景
	sceneManager.switchScene<MainMenuScene>();

	QuadMesh quadMesh;
	float lastTime = static_cast<float>(glfwGetTime());

	// 9. 主邏輯與渲染迴圈
	while (!appWindow->shouldClose()) {
		float currentTime = static_cast<float>(glfwGetTime());
		float dt = currentTime - lastTime;
		lastTime = currentTime;

		appWindow->pollEvents();
		sceneManager.update(dt);

		glClearColor(0.12f, 0.12f, 0.14f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		uiShader.use();
		glBindVertexArray(quadMesh.getVAO());
		sceneManager.render(uiShader);

		appWindow->swapBuffers();
	}

	return 0;
}
