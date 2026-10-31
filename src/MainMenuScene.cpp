#include <Scene/MainMenuScene.h>
#include <Scene/SceneManager.h>
#include <UI/UIButton.h>
#include <UI/UIText.h>
#include <iostream>

// 前置宣告遊戲場景
#include <Scene/GameScene.h>

void MainMenuScene::onEnter() {
    std::cout << "[Scene] Entering MainMenuScene...\n";

    FontLoader& font = m_resources.getFont("default");

    // 放置一個在父容器水平 50.0%、垂直 80.0% 的按鈕
    auto* btn = m_uiRoot.emplaceChild<UIButton>(0.0f, 0.0f, 200.0f, 50.0f);
    btn->setPositionPercent(80.0f, 95.0f);

    // 精確到 0.01%
    auto* text = m_uiRoot.emplaceChild<UIText>(0.0f, 0.0f, font, U"精確座標");
    text->setPositionPercent(80.0f, 90.0f);

    // 場景專屬標題
    auto* title = m_uiRoot.emplaceChild<UIText>(300.0f, 150.0f, font, U"主選單");
    title->setColor(0.95f, 0.85f, 0.20f, 1.0f);

    // 開始遊戲按鈕：點擊後切換到 GameScene
    auto* btnStart = m_uiRoot.emplaceChild<UIButton>(300.0f, 260.0f, 200.0f, 60.0f);
    btnStart->setOnClick([this]() {
        m_manager.switchScene<GameScene>();
    });

    auto* startLabel = m_uiRoot.emplaceChild<UIText>(0.0f, 0.0f, font, U"開始");
    startLabel->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    float textX = btnStart->getX() + (btnStart->getWidth() - startLabel->getWidth()) * 0.5f;
    float textY = btnStart->getY() + (btnStart->getHeight() - startLabel->getHeight()) * 0.5f;
    startLabel->setPosition(textX, textY);
}

void MainMenuScene::onExit() {
    std::cout << "[Scene] Exiting MainMenuScene (UI completely destroyed).\n";
    m_uiRoot.clear(); // 清理局部 UI
}
