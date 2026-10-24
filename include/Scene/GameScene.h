#ifndef GIRIGARA_SCENE_GAME_SCENE_H
#define GIRIGARA_SCENE_GAME_SCENE_H

#include <Scene/Scene.h>
#include <UI/UIWindow.h>

class GameScene : public Scene {
public:
    using Scene::Scene;
    ~GameScene() override = default;

    void onEnter() override;
    void onExit() override;

private:
    UIWindow* m_consoleWindow = nullptr;
};

#endif // GIRIGARA_SCENE_GAME_SCENE_H
