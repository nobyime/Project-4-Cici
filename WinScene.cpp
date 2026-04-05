#include "WinScene.h"

extern int gLives;
extern bool gBeatBoss;

WinScene::WinScene()                                      : Scene { {0.0f}, nullptr   } {}
WinScene::WinScene(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

WinScene::~WinScene() {}

void WinScene::initialise()
{
    mGameState.player = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = -1;
}

void WinScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        gLives = 3;
        mGameState.nextSceneID = 0;   // back to MenuScene
    }
    if (!gBeatBoss && IsKeyPressed(KEY_FOUR))
    {
        gLives = 3;
        mGameState.nextSceneID = 4;   // Level4
    }
}

void WinScene::render()
{
    DrawText("YOU WIN", 385, 250, 55, GREEN);
    if (gBeatBoss)
    {
        DrawText("Press ENTER to return to menu", 310, 310, 25, WHITE);
    }
    else
    {
        DrawText("Press ENTER to return to menu", 310, 310, 25, WHITE);
        DrawText("Press 4 to challenge the boss", 320, 335, 25, WHITE);
    }
}

void WinScene::shutdown()
{
}