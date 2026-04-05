#include "Entity.h"

#ifndef SCENE_H
#define SCENE_H

struct Weapon
{
    Texture2D texture;

    Vector2 position;
    Vector2 velocity;
    Vector2 scale;
    Vector2 atlasDimensions;

    int currentFrame = 0;
    int frameCount   = 5;
    float animationTime = 0.0f;
    float frameSpeed    = 0.08f;

    float hoverPhase = 0.0f;
    float maxDistance = 280.0f;
    Vector2 throwStartPosition = { 0.0f, 0.0f };

    bool isThrown    = false;
    bool isReturning = false;
    bool hasHitBossOnThrow = false;
    bool hasHitBossOnReturn = false;
};

struct GameState
{
    Entity *player;
    Map *map;

    Music bgm;
    Sound jumpSound;
    Sound walkSound;

    Weapon weapon[3];
    bool facingLeft = false;

    int activeWeaponCount = 1;
    
    int nextSceneID;
};

class Scene
{
protected:
    GameState mGameState;
    Vector2 mOrigin;
    const char *mBGColourHexCode = "#000000";

public:
    Scene();
    Scene(Vector2 origin, const char *bgHexCode);

    virtual void initialise() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void shutdown() = 0;
    virtual void renderUI() {}

    GameState&   getState()                { return mGameState; }
    const GameState& getState()      const { return mGameState; }
    Vector2     getOrigin()          const { return mOrigin;    }
    const char* getBGColourHexCode() const { return mBGColourHexCode; }
};

#endif
