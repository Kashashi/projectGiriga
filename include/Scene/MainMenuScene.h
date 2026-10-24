#ifndef GIRIGARA_SCENE_MAIN_MENU_SCENE_H
#define GIRIGARA_SCENE_MAIN_MENU_SCENE_H

#include <Scene/Scene.h>

class MainMenuScene : public Scene {
public:
    using Scene::Scene;
    ~MainMenuScene() override = default;

    void onEnter() override;
    void onExit() override;
};

#endif // GIRIGARA_SCENE_MAIN_MENU_SCENE_H
