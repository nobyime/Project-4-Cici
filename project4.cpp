/**
* Author: Xiling Wang
* Assignment: Rise of the AI
* Date due: 2026-04-04, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#include "CS3113/Entity.h"

#include "MenuScene.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"
#include "Level4.h"
#include "WinScene.h"
#include "LoseScene.h"

constexpr int SCREEN_WIDTH  = 1000,
              SCREEN_HEIGHT = 600,
              FPS           = 120,
              NUMBER_OF_SCENES = 7;

constexpr Vector2 ORIGIN = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

AppStatus gAppStatus   = RUNNING;
float gPreviousTicks   = 0.0f,
      gTimeAccumulator = 0.0f,
      gWalkSoundTimer  = 0.0f;

bool gBeatBoss = false;
Camera2D gCamera = {0};

Scene *gCurrentScene = nullptr;
std::vector<Scene*> gScenes = {};

MenuScene *gMenuScene = nullptr;
Level1 *gLevel1 = nullptr;
Level2 *gLevel2 = nullptr;
Level3 *gLevel3 = nullptr;
Level4 *gLevel4 = nullptr;
WinScene *gWinScene = nullptr;
LoseScene *gLoseScene = nullptr;

Texture2D gBackground;
Music gBgm;

int gLives = 3;

void switchToScene(Scene *scene);
void initialise();
void processInput();
void update();
void render();
void shutdown();

void switchToScene(Scene *scene)
{
    if (gCurrentScene != nullptr && gCurrentScene != scene)
        gCurrentScene->shutdown();

    gCurrentScene = scene;
    gCurrentScene->initialise();

    if (gCurrentScene->getState().player != nullptr)
        gCamera.target = gCurrentScene->getState().player->getPosition();
}

void throwWeapon()
{
    for (int i = 0; i < gCurrentScene->getState().activeWeaponCount; i++)
    {
        if (!gCurrentScene->getState().weapon[i].isThrown &&
            !gCurrentScene->getState().weapon[i].isReturning)
        {
            gCurrentScene->getState().weapon[i].isThrown = true;
            gCurrentScene->getState().weapon[i].isReturning = false;
            gCurrentScene->getState().weapon[i].throwStartPosition =
                gCurrentScene->getState().weapon[i].position;
            gCurrentScene->getState().weapon[i].hasHitBossOnThrow = false;
            gCurrentScene->getState().weapon[i].hasHitBossOnReturn = false;

            float throwSpeed = 560.0f;

            if (gCurrentScene->getState().facingLeft)
                gCurrentScene->getState().weapon[i].velocity = { -throwSpeed, 0.0f };
            else
                gCurrentScene->getState().weapon[i].velocity = { throwSpeed, 0.0f };

            break;
        }
    }
}

void initialise()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Rise of the AI");
    InitAudioDevice();
    gBackground = LoadTexture("images/bg.png");

    gBgm = LoadMusicStream("bgm/Lord of the Rangs.wav");
    SetMusicVolume(gBgm, 0.15f);
    PlayMusicStream(gBgm);

    gMenuScene = new MenuScene(ORIGIN, "#ffffffff");
    gLevel1    = new Level1(ORIGIN, "#ffffffff");
    gLevel2    = new Level2(ORIGIN, "#ffffffff");
    gLevel3    = new Level3(ORIGIN, "#ffffffff");
    gLevel4    = new Level4(ORIGIN, "#ffffffff");
    gWinScene  = new WinScene(ORIGIN, "#ffffffff");
    gLoseScene = new LoseScene(ORIGIN, "#ffffffff");

    gScenes.push_back(gMenuScene);
    gScenes.push_back(gLevel1);
    gScenes.push_back(gLevel2);
    gScenes.push_back(gLevel3);
    gScenes.push_back(gLevel4);
    gScenes.push_back(gWinScene);
    gScenes.push_back(gLoseScene);

    switchToScene(gScenes[0]);

    gCamera.offset   = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f + 90.0f };
    gCamera.rotation = 0.0f;
    gCamera.zoom     = 1.0f;

    SetTargetFPS(FPS);
}

void processInput()
{
    if (gCurrentScene->getState().player != nullptr)
    {
        gCurrentScene->getState().player->resetMovement();

        if (IsKeyDown(KEY_A))
        {
            gCurrentScene->getState().player->moveLeft();
            gCurrentScene->getState().facingLeft = true;
        }

        if (IsKeyDown(KEY_D))
        {
            gCurrentScene->getState().player->moveRight();
            gCurrentScene->getState().facingLeft = false;
        }

        if (IsKeyPressed(KEY_W) &&
            gCurrentScene->getState().player->isCollidingBottom())
        {
            gCurrentScene->getState().player->jump();
            PlaySound(gCurrentScene->getState().jumpSound);
        }

        if (GetLength(gCurrentScene->getState().player->getMovement()) > 1.0f)
            gCurrentScene->getState().player->normaliseMovement();

        bool isMovingHorizontally = IsKeyDown(KEY_A) || IsKeyDown(KEY_D);

        bool isOnGround = gCurrentScene->getState().player->isCollidingBottom();

        if (isMovingHorizontally && isOnGround)
        {
            if (gWalkSoundTimer <= 0.0f)
            {
                PlaySound(gCurrentScene->getState().walkSound);
                gWalkSoundTimer = 1.0f;
            }

            gWalkSoundTimer -= GetFrameTime();
        }
        else
        {
            gWalkSoundTimer = 0.0f;
            StopSound(gCurrentScene->getState().walkSound);
        }

        if (IsKeyPressed(KEY_J))
        {
            if (gCurrentScene == gLevel2)
                gLevel2->throwWeaponAtNearestBat();
            else if (gCurrentScene == gLevel3)
                gLevel3->throwWeaponAtNearestSlime();
            else
                throwWeapon();
        }
    }

    if (IsKeyPressed(KEY_ONE))   switchToScene(gScenes[1]);
    if (IsKeyPressed(KEY_TWO))   switchToScene(gScenes[2]);
    if (IsKeyPressed(KEY_THREE)) switchToScene(gScenes[3]);
    if (IsKeyPressed(KEY_FOUR))  switchToScene(gScenes[4]);
    if (IsKeyPressed(KEY_Q) || WindowShouldClose()) gAppStatus = TERMINATED;
}

void update()
{
    float ticks = (float) GetTime();
    float deltaTime = ticks - gPreviousTicks;
    gPreviousTicks  = ticks;

    deltaTime += gTimeAccumulator;

    if (deltaTime < FIXED_TIMESTEP)
    {
        gTimeAccumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP)
    {
        UpdateMusicStream(gBgm);
        gCurrentScene->update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;

        if (gCurrentScene->getState().player != nullptr)
            gCamera.target = gCurrentScene->getState().player->getPosition();
    }

    gTimeAccumulator = deltaTime;

    if (gCurrentScene->getState().nextSceneID >= 0)
    {
        int i = gCurrentScene->getState().nextSceneID;

        if (i >= 0 && i < NUMBER_OF_SCENES)
            switchToScene(gScenes[i]);
    }
}

void render()
{
    BeginDrawing();

    DrawTexturePro(
        gBackground,
        { 0, 0, (float)gBackground.width, (float)gBackground.height },
        { 0, 0, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT },
        { 0, 0 },
        0.0f,
        WHITE
    );

    if (gCurrentScene->getState().player != nullptr)
    {
        BeginMode2D(gCamera);
        gCurrentScene->render();
        EndMode2D();
        gCurrentScene->renderUI();
    }
    else
    {
        gCurrentScene->render();
        gCurrentScene->renderUI();
    }

    EndDrawing();
}

void shutdown()
{
    delete gMenuScene;
    delete gLevel1;
    delete gLevel2;
    delete gLevel3;
    delete gLevel4;
    delete gWinScene;
    delete gLoseScene;

    gMenuScene = nullptr;
    gLevel1    = nullptr;
    gLevel2    = nullptr;
    gLevel3    = nullptr;
    gLevel4    = nullptr;
    gWinScene  = nullptr;
    gLoseScene = nullptr;
    
    gScenes.clear();
    UnloadTexture(gBackground);
    UnloadMusicStream(gBgm);
    CloseAudioDevice();
    CloseWindow();
}

int main()
{
    initialise();

    while (gAppStatus == RUNNING)
    {
        processInput();
        update();
        render();
    }

    shutdown();
    return 0;
}