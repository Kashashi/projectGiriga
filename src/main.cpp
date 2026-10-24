#include <Girigara.h>
#include <Scene/MainMenuScene.h>

int main() {
	// 實例化超級類別 (自動初始化所有子系統與全域 UI)
	Girigara engine(800, 600, "Project Girigara");

	// 切換至初始場景
	engine.switchScene<MainMenuScene>();

	// 進入主迴圈
	engine.run();

	return 0;
}
