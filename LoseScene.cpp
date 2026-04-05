#include "LoseScene.h"

extern int gLives;

LoseScene::LoseScene()                                      : Scene { {0.0f}, nullptr   } {}
LoseScene::LoseScene(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

LoseScene::~LoseScene() {}

void LoseScene::initialise()
{
    mGameState.player = nullptr;
    mGameState.map = nullptr;
    mGameState.nextSceneID = -1;
}

void LoseScene::update(float deltaTime)
{
    if (IsKeyPressed(KEY_ENTER))
    {
        gLives = 3;
        mGameState.nextSceneID = 0;   // back to MenuScene
    }
}

void LoseScene::render()
{
    DrawText("YOU DIE...", 380, 250, 55, BLACK);
    DrawText("Press ENTER to return to menu", 250, 320, 30, WHITE);
}

void LoseScene::shutdown()
{
}