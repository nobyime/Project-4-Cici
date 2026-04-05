#include "MenuScene.h"

extern int gLives;

MenuScene::MenuScene()                                      : Scene { {0.0f}, nullptr   } {}
MenuScene::MenuScene(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

MenuScene::~MenuScene() {}

void MenuScene::initialise()
{
    mGameState.player = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = -1;
}

void MenuScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        gLives = 3;
        mGameState.nextSceneID = 1;   // Level1
    }
}

void MenuScene::render()
{
    DrawText("Spin That Boomerang!", 340, 270, 30, WHITE);
    DrawText("(press ENTER to start)", 382, 300, 20, WHITE);
}

void MenuScene::shutdown(){}