#include "Level4.h"

extern int gLives;
extern bool gBeatBoss;

Level4::Level4()                                      : Scene { {0.0f}, nullptr   } {}
Level4::Level4(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level4::~Level4() { }

void Level4::initialiseWeapons()
{
    for (int i = 0; i < mGameState.activeWeaponCount; i++)
    {
        mGameState.weapon[i].texture = LoadTexture("images/weapon.png");
        mGameState.weapon[i].position = { 0.0f, 0.0f };
        mGameState.weapon[i].velocity = { 0.0f, 0.0f };
        mGameState.weapon[i].scale = { 55.0f, 55.0f };
        mGameState.weapon[i].atlasDimensions = { 5, 1 };

        mGameState.weapon[i].currentFrame = 0;
        mGameState.weapon[i].frameCount   = 5;
        mGameState.weapon[i].animationTime = 0.0f;
        mGameState.weapon[i].frameSpeed    = 0.08f;

        mGameState.weapon[i].hoverPhase = i * 0.8f;
        mGameState.weapon[i].maxDistance = 280.0f;
        mGameState.weapon[i].throwStartPosition = { 0.0f, 0.0f };

        mGameState.weapon[i].isThrown    = false;
        mGameState.weapon[i].isReturning = false;
        mGameState.weapon[i].hasHitBossOnThrow = false;
        mGameState.weapon[i].hasHitBossOnReturn = false;
    }
}

void Level4::renderWeapon(Weapon *weapon)
{
    float frameWidth  = (float) weapon->texture.width  / weapon->atlasDimensions.x;
    float frameHeight = (float) weapon->texture.height / weapon->atlasDimensions.y;

    Rectangle sourceArea = {
        frameWidth * weapon->currentFrame,
        0.0f,
        frameWidth,
        frameHeight
    };

    Rectangle destinationArea = {
        weapon->position.x,
        weapon->position.y,
        weapon->scale.x,
        weapon->scale.y
    };

    DrawTexturePro(
        weapon->texture,
        sourceArea,
        destinationArea,
        { weapon->scale.x / 2.0f, weapon->scale.y / 2.0f },
        0.0f,
        WHITE
    );
}

void Level4::updateWeapons(float deltaTime)
{
    Vector2 currentMePosition = mGameState.player->getPosition();

    float sideOffsetX     = 70.0f;
    float verticalSpacing = 52.0f;
    float hoverAmplitude  = 10.0f;
    float hoverSpeed      = 3.2f;

    for (int i = 0; i < mGameState.activeWeaponCount; i++)
    {
        mGameState.weapon[i].animationTime += deltaTime;
        if (mGameState.weapon[i].animationTime >= mGameState.weapon[i].frameSpeed)
        {
            mGameState.weapon[i].animationTime = 0.0f;
            mGameState.weapon[i].currentFrame =
                (mGameState.weapon[i].currentFrame + 1) % mGameState.weapon[i].frameCount;
        }

        if (!mGameState.weapon[i].isThrown && !mGameState.weapon[i].isReturning)
        {
            float xOffset = mGameState.facingLeft ? -sideOffsetX : sideOffsetX;
            float centerIndex = (mGameState.activeWeaponCount - 1) / 2.0f;
            float baseYOffset = (i - centerIndex) * verticalSpacing;
            float hoverYOffset =
                sinf((float)GetTime() * hoverSpeed + mGameState.weapon[i].hoverPhase)
                * hoverAmplitude;

            mGameState.weapon[i].position = {
                currentMePosition.x + xOffset,
                currentMePosition.y + baseYOffset + hoverYOffset
            };
        }
        else if (mGameState.weapon[i].isThrown)
        {
            mGameState.weapon[i].position.x += mGameState.weapon[i].velocity.x * deltaTime;
            mGameState.weapon[i].position.y += mGameState.weapon[i].velocity.y * deltaTime;

            float distance =
                Vector2Distance(
                mGameState.weapon[i].position,
                mGameState.weapon[i].throwStartPosition
                );

            if (distance >= mGameState.weapon[i].maxDistance)
            {
                mGameState.weapon[i].isThrown = false;
                mGameState.weapon[i].isReturning = true;
            }
        }
        else if (mGameState.weapon[i].isReturning)
        {
            float xOffset = mGameState.facingLeft ? -sideOffsetX : sideOffsetX;
            float centerIndex = (mGameState.activeWeaponCount - 1) / 2.0f;
            float baseYOffset = (i - centerIndex) * verticalSpacing;

            Vector2 targetPosition = {
                currentMePosition.x + xOffset,
                currentMePosition.y + baseYOffset
            };

            Vector2 direction = {
                targetPosition.x - mGameState.weapon[i].position.x,
                targetPosition.y - mGameState.weapon[i].position.y
            };

            float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

            if (length < 12.0f)
            {
                mGameState.weapon[i].isReturning = false;
                mGameState.weapon[i].position = targetPosition;
                mGameState.weapon[i].velocity = { 0.0f, 0.0f };
                mGameState.weapon[i].hasHitBossOnThrow = false;
                mGameState.weapon[i].hasHitBossOnReturn = false;
            }
            else
            {
                direction.x /= length;
                direction.y /= length;

                float returnSpeed = 620.0f;

                mGameState.weapon[i].position.x += direction.x * returnSpeed * deltaTime;
                mGameState.weapon[i].position.y += direction.y * returnSpeed * deltaTime;
            }
        }
    }
}

float Level4::getGroundYAtX(float x)
{
    for (float y = mOrigin.y - 400.0f; y < mOrigin.y + 500.0f; y += 4.0f)
    {
    Vector2 probe = { x, y };
      float xOverlap = 0.0f;
      float yOverlap = 0.0f;

    if (mGameState.map->isSolidTileAt(probe, &xOverlap, &yOverlap))
        return y - yOverlap;
    }

    return mOrigin.y - 150.0f;
}

void Level4::initialiseBoss()
{
    mBoss.position = { mOrigin.x, getGroundYAtX(mOrigin.x) - 35.0f};
    mBoss.velocity = { 0.0f, 0.0f };
    mBoss.scale = { BOSS_SCALE_X, BOSS_SCALE_Y };

    mBoss.currentFrame = 0;
    mBoss.animationTime = 0.0f;
    mBoss.frameSpeed = 0.10f;

    mBoss.active = true;
    mBoss.facingLeft = true;
    mBoss.hasDamagedPlayer = false;
    mBoss.state = BOSS_WALKING;

    mBoss.hp = BOSS_MAX_HP;
    mBoss.attackTimer = 0.0f;
    mBoss.attackCooldown = (float)GetRandomValue(15, 30) / 10.0f;
}

void Level4::initialise()
{
    mGameState.nextSceneID = -1;
    mGameState.facingLeft = false;

    mGameState.jumpSound = LoadSound("bgm/jump_on_grass.wav");
    SetSoundVolume(mGameState.jumpSound, 0.3f);

    mGameState.walkSound = LoadSound("bgm/walk_on_grass.wav");
    SetSoundVolume(mGameState.walkSound, 1.5f);

    mGameState.map = new Map(
        LEVEL4_WIDTH, LEVEL4_HEIGHT,
        (unsigned int *) mLevelData,
        "images/tile.png",
        TILE_DIMENSION,
        7, 1,
        mOrigin
    );

    std::map<Direction, std::vector<int>> playerAnimationAtlas = {
        {LEFT,  {0, 1, 2, 3, 4, 5, 6, 7, 8}},
        {RIGHT, {9, 10, 11, 12, 13, 14, 15, 16, 17}}
    };

    mGameState.player = new Entity(
        {mOrigin.x - 450.0f, mOrigin.y - 150.0f},
        {100.0f, 100.0f},
        "images/me.png",
        ATLAS,
        { 9, 2 },
        playerAnimationAtlas,
        PLAYER
    );

    mGameState.player->setJumpingPower(550.0f);
    mGameState.player->setColliderDimensions({
        mGameState.player->getScale().x / 3.5f,
        mGameState.player->getScale().y / 3.0f
    });
    mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

    mGameState.activeWeaponCount = 3;

    initialiseWeapons();

    mBossWalkTexture   = LoadTexture("images/boss_walk.png");
    mBossAttackTexture = LoadTexture("images/boss_attack.png");
    mBossDeathTexture  = LoadTexture("images/boss_death.png");
    mHeartTexture      = LoadTexture("images/heart.png");

    initialiseBoss();
}

bool Level4::weaponHitsBoss(Weapon *weapon)
{
    float dx = fabs(weapon->position.x - mBoss.position.x);
    float dy = fabs(weapon->position.y - mBoss.position.y);

    return (dx < mBoss.scale.x * 0.25f && dy < mBoss.scale.y * 0.25f);
}

void Level4::updateBoss(float deltaTime)
{
    if (!mBoss.active) return;

    for (int w = 0; w < mGameState.activeWeaponCount; w++)
    {
        Weapon *weapon = &mGameState.weapon[w];

        if (!weaponHitsBoss(weapon)) continue;
        if (mBoss.state == BOSS_DYING) continue;

        if (weapon->isThrown && !weapon->hasHitBossOnThrow)
        {
            mBoss.hp -= WEAPON_DAMAGE;
            if (mBoss.hp < 0) mBoss.hp = 0;
            weapon->hasHitBossOnThrow = true;
        }
        else if (weapon->isReturning && !weapon->hasHitBossOnReturn)
        {
            mBoss.hp -= WEAPON_DAMAGE;
            if (mBoss.hp < 0) mBoss.hp = 0;
            weapon->hasHitBossOnReturn = true;
        }
    }

    if (mBoss.hp <= 0 && mBoss.state != BOSS_DYING)
    {
        mBoss.state = BOSS_DYING;
        mBoss.currentFrame = 0;
        mBoss.animationTime = 0.0f;
        mBoss.frameSpeed = 0.10f;
        mBoss.velocity = { 0.0f, 0.0f };
    }

    Vector2 playerPos = mGameState.player->getPosition();

    if (playerPos.x < mBoss.position.x) mBoss.facingLeft = true;
    else                                mBoss.facingLeft = false;

    if (mBoss.state == BOSS_DYING)
    {
        mBoss.animationTime += deltaTime;
        if (mBoss.animationTime >= mBoss.frameSpeed)
        {
            mBoss.animationTime = 0.0f;
            mBoss.currentFrame++;
        }

        if (mBoss.currentFrame >= BOSS_DEATH_FRAMES)
        {
            mBoss.active = false;
            gBeatBoss = true;
            mGameState.nextSceneID = 5; // WinScene
        }

        return;
    }

    if (mBoss.state == BOSS_ATTACKING)
    {
        mBoss.animationTime += deltaTime;
        if (mBoss.animationTime >= mBoss.frameSpeed)
        {
            mBoss.animationTime = 0.0f;
            mBoss.currentFrame++;

            if (mBoss.currentFrame == 3 && !mBoss.hasDamagedPlayer)
            {
                float dx = fabs(playerPos.x - mBoss.position.x);
                float dy = fabs(playerPos.y - mBoss.position.y);

                if (dx < mBoss.scale.x / 2.0f && dy < mBoss.scale.y / 2.0f)
                {
                gLives--;
                mBoss.hasDamagedPlayer = true;

                if (gLives <= 0)
                    mGameState.nextSceneID = 6; // LoseScene
                }
            }

            if (mBoss.currentFrame >= BOSS_ATTACK_FRAMES)
            {
                mBoss.state = BOSS_WALKING;
                mBoss.currentFrame = 0;
                mBoss.animationTime = 0.0f;
                mBoss.frameSpeed = 0.10f;
                mBoss.hasDamagedPlayer = false;
                mBoss.attackTimer = 0.0f;
                mBoss.attackCooldown = (float)GetRandomValue(15, 30) / 10.0f;
            }
        }

        return;
    }

    mBoss.attackTimer += deltaTime;

    if (mBoss.attackTimer >= mBoss.attackCooldown)
    {
        mBoss.state = BOSS_ATTACKING;
        mBoss.currentFrame = 0;
        mBoss.animationTime = 0.0f;
        mBoss.frameSpeed = 0.10f;
        mBoss.hasDamagedPlayer = false;
        mBoss.velocity = { 0.0f, 0.0f };
        return;
    }

    if (playerPos.x < mBoss.position.x)
        mBoss.velocity.x = -BOSS_MOVE_SPEED;
    else
        mBoss.velocity.x = BOSS_MOVE_SPEED;

    mBoss.position.x += mBoss.velocity.x * deltaTime;
    mBoss.position.y = getGroundYAtX(mBoss.position.x) - 35.0f;

    mBoss.animationTime += deltaTime;
    if (mBoss.animationTime >= mBoss.frameSpeed)
    {
        mBoss.animationTime = 0.0f;
        mBoss.currentFrame++;

        if (mBoss.currentFrame >= BOSS_WALK_FRAMES)
            mBoss.currentFrame = 0;
    }
}

void Level4::renderBoss()
{
    if (!mBoss.active) return;

    Texture2D texture;
    int frameCount;

    if (mBoss.state == BOSS_WALKING)
    {
        texture = mBossWalkTexture;
        frameCount = BOSS_WALK_FRAMES;
    }
    else if (mBoss.state == BOSS_ATTACKING)
    {
        texture = mBossAttackTexture;
        frameCount = BOSS_ATTACK_FRAMES;
    }
    else
    {
        texture = mBossDeathTexture;
        frameCount = BOSS_DEATH_FRAMES;
    }

    float frameWidth  = (float)texture.width / frameCount;
    float frameHeight = (float)texture.height / 2.0f;

    int row = mBoss.facingLeft ? 1 : 0;

    Rectangle sourceArea = {
        frameWidth * mBoss.currentFrame,
        frameHeight * row,
        frameWidth,
        frameHeight
    };

    Rectangle destinationArea = {
        mBoss.position.x,
        mBoss.position.y,
        mBoss.scale.x,
        mBoss.scale.y
    };

    DrawTexturePro(
        texture,
        sourceArea,
        destinationArea,
        { mBoss.scale.x / 2.0f, mBoss.scale.y / 2.0f },
        0.0f,
        WHITE
    );
    }

    void Level4::update(float deltaTime)
    {
    mGameState.player->update(
        deltaTime,
        nullptr,
        mGameState.map,
        nullptr,
        0
    );

    updateWeapons(deltaTime);
    updateBoss(deltaTime);

    if (mGameState.player->getPosition().y > 800.0f)
    {
        gLives--;

        if (gLives <= 0) mGameState.nextSceneID = 6;
        else             mGameState.nextSceneID = 4;
    }
}

void Level4::render()
{
mGameState.map->render();
    renderBoss();

    for (int i = 0; i < mGameState.activeWeaponCount; i++)
        renderWeapon(&mGameState.weapon[i]);

    mGameState.player->render();
    }

    void Level4::renderUI()
    {
    DrawText("LEVEL 4", 20, 20, 30, WHITE);

    float heartFrameWidth  = (float)mHeartTexture.width / 2.0f;
    float heartFrameHeight = (float)mHeartTexture.height;

    float heartSize = 36.0f;
    float startX = 20.0f;
    float startY = 50.0f;
    float spacing = 42.0f;

    for (int i = 0; i < 3; i++)
    {
        bool isAlive = (i < gLives);

        Rectangle sourceArea = {
            isAlive ? 0.0f : heartFrameWidth,
            0.0f,
            heartFrameWidth,
            heartFrameHeight
        };

        Rectangle destinationArea = {
            startX + i * spacing,
            startY,
            heartSize,
            heartSize
        };

        DrawTexturePro(
            mHeartTexture,
            sourceArea,
            destinationArea,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    float barWidth = 360.0f;
    float barHeight = 28.0f;
    float barX = (1000.0f - barWidth) / 2.0f;
    float barY = 20.0f;

    DrawRectangle((int)barX - 2, (int)barY - 2, (int)barWidth + 4, (int)barHeight + 4, BLACK);
    DrawRectangle((int)barX, (int)barY, (int)barWidth, (int)barHeight, DARKGRAY);

    float percent = (float)mBoss.hp / (float)BOSS_MAX_HP;
    if (percent < 0.0f) percent = 0.0f;

    DrawRectangle((int)barX, (int)barY, (int)(barWidth * percent), (int)barHeight, RED);

    DrawText(
        TextFormat("%d / %d", mBoss.hp, BOSS_MAX_HP),
        (int)(barX + 115.0f),
        (int)(barY + 3.0f),
        20,
        WHITE
    );
}

void Level4::shutdown()
{
    if (mGameState.player != nullptr)
    {
        delete mGameState.player;
        mGameState.player = nullptr;
    }

    if (mGameState.map != nullptr)
    {
        delete mGameState.map;
        mGameState.map = nullptr;
    }

    for (int i = 0; i < mGameState.activeWeaponCount; i++)
        UnloadTexture(mGameState.weapon[i].texture);

    UnloadSound(mGameState.jumpSound);
    UnloadSound(mGameState.walkSound);

    UnloadTexture(mBossWalkTexture);
    UnloadTexture(mBossAttackTexture);
    UnloadTexture(mBossDeathTexture);
    UnloadTexture(mHeartTexture);
}