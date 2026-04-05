#include "Level2.h"

extern int gLives;
extern bool gBeatBoss;

Level2::Level2()                                      : Scene { {0.0f}, nullptr   } {}
Level2::Level2(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level2::~Level2() { }

void Level2::initialiseWeapons()
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
   }
}

void Level2::initialiseBats()
{
   for (int i = 0; i < MAX_BATS; i++)
   {
      mBats[i].position = { 0.0f, 0.0f };
      mBats[i].velocity = { 0.0f, 0.0f };
      mBats[i].scale = { 100.0f, 100.0f };

      mBats[i].currentFrame = 0;
      mBats[i].animationTime = 0.0f;
      mBats[i].frameSpeed = 0.08f;

      mBats[i].active = false;
      mBats[i].facingLeft = true;
      mBats[i].hasDamagedPlayer = false;
      mBats[i].state = BAT_FLYING;
   }
}

void Level2::initialise()
{
   mGameState.nextSceneID = -1;
   mGameState.facingLeft = false;

   mGameState.jumpSound = LoadSound("bgm/jump_on_grass.wav");
   SetSoundVolume(mGameState.jumpSound, 0.3f);

   mGameState.walkSound = LoadSound("bgm/walk_on_grass.wav");
   SetSoundVolume(mGameState.walkSound, 1.5f);

   mGameState.map = new Map(
      LEVEL2_WIDTH, LEVEL2_HEIGHT,
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

   mGameState.activeWeaponCount = 2;

   initialiseWeapons();

   mBatFlyTexture    = LoadTexture("images/bat_fly.png");
   mBatAttackTexture = LoadTexture("images/bat_attack.png");
   mBatDeathTexture  = LoadTexture("images/bat_death.png");
   mHeartTexture     = LoadTexture("images/heart.png");

   initialiseBats();

   mBatSpawnTimer = 0.0f;
   mBatKillCount = 0;
   mBatKillTarget = 20;
}

void Level2::spawnBat()
{
   for (int i = 0; i < MAX_BATS; i++)
   {
      if (!mBats[i].active)
      {
         mBats[i].active = true;
         mBats[i].state = BAT_FLYING;
         mBats[i].currentFrame = 0;
         mBats[i].animationTime = 0.0f;
         mBats[i].frameSpeed = 0.08f;
         mBats[i].hasDamagedPlayer = false;

         float a = BAT_SPAWN_HALF_CHORD;
         float h = BAT_SPAWN_ARC_HEIGHT;

         float radius = (a * a + h * h) / (2.0f * h);
         float centerYOffset = radius - h;

         Vector2 circleCenter = {
            mOrigin.x,
            mOrigin.y + centerYOffset
         };

         float leftAngle = atan2f(
            mOrigin.y - circleCenter.y,
            (mOrigin.x - a) - circleCenter.x
         );

         float rightAngle = atan2f(
            mOrigin.y - circleCenter.y,
            (mOrigin.x + a) - circleCenter.x
         );

         float t = (float)GetRandomValue(0, 10000) / 10000.0f;
         float angle = leftAngle + t * (rightAngle - leftAngle);

         mBats[i].position = {
            circleCenter.x + cosf(angle) * radius,
            circleCenter.y + sinf(angle) * radius -200.0f
         };

         mBats[i].facingLeft = (mGameState.player->getPosition().x < mBats[i].position.x);

         break;
      }
   }
}

bool Level2::weaponHitsBat(Weapon *weapon, Bat *bat)
{
   float dx = fabs(weapon->position.x - bat->position.x);
   float dy = fabs(weapon->position.y - bat->position.y);

   return (dx < 45.0f && dy < 45.0f);
}

bool Level2::isBatBlockedAtPosition(Bat *bat, Vector2 position)
{
   float halfW = bat->scale.x / 2.0f;
   float halfH = bat->scale.y / 2.0f;

   float xDir = 0.0f;
   float yDir = 0.0f;

   float velocityLength = sqrtf(bat->velocity.x * bat->velocity.x +
                                bat->velocity.y * bat->velocity.y);

   if (velocityLength > 0.0f)
   {
      xDir = bat->velocity.x / velocityLength;
      yDir = bat->velocity.y / velocityLength;
   }

   float probeX = position.x + xDir * halfW * 0.8f;
   float probeY = position.y + yDir * halfH * 0.8f;

   Vector2 probeCenter = { probeX, probeY };
   Vector2 probeUpper  = { probeX, probeY - halfH * 0.5f };
   Vector2 probeLower  = { probeX, probeY + halfH * 0.5f };

   float xOverlap = 0.0f;
   float yOverlap = 0.0f;

   if (mGameState.map->isSolidTileAt(probeCenter, &xOverlap, &yOverlap)) return true;
   if (mGameState.map->isSolidTileAt(probeUpper,  &xOverlap, &yOverlap)) return true;
   if (mGameState.map->isSolidTileAt(probeLower,  &xOverlap, &yOverlap)) return true;

   return false;
}

Bat* Level2::findNearestTargetOnFacingSide()
{
   Bat *nearest = nullptr;
   float bestDistance = 999999.0f;

   Vector2 playerPos = mGameState.player->getPosition();
   bool facingLeft = mGameState.facingLeft;

   for (int i = 0; i < MAX_BATS; i++)
   {
      if (!mBats[i].active) continue;
      if (mBats[i].state == BAT_DEAD) continue;

      if (facingLeft && mBats[i].position.x >= playerPos.x) continue;
      if (!facingLeft && mBats[i].position.x <= playerPos.x) continue;

      float distance = Vector2Distance(playerPos, mBats[i].position);

      if (distance < bestDistance)
      {
         bestDistance = distance;
         nearest = &mBats[i];
      }
   }

   return nearest;
}

void Level2::throwWeaponAtNearestBat()
{
   Bat *target = findNearestTargetOnFacingSide();
   if (target == nullptr) return;

   for (int i = 0; i < mGameState.activeWeaponCount; i++)
   {
      if (!mGameState.weapon[i].isThrown &&
          !mGameState.weapon[i].isReturning)
      {
         mGameState.weapon[i].isThrown = true;
         mGameState.weapon[i].isReturning = false;
         mGameState.weapon[i].throwStartPosition =
            mGameState.weapon[i].position;

         Vector2 direction = {
            target->position.x - mGameState.weapon[i].position.x,
            target->position.y - mGameState.weapon[i].position.y
         };

         float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
         if (length == 0.0f) return;

         direction.x /= length;
         direction.y /= length;

         float throwSpeed = 560.0f;

         mGameState.weapon[i].velocity = {
            direction.x * throwSpeed,
            direction.y * throwSpeed
         };

         break;
      }
   }
}

void Level2::updateWeapons(float deltaTime)
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

void Level2::renderWeapon(Weapon *weapon)
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

void Level2::updateBats(float deltaTime)
{
   mBatSpawnTimer += deltaTime;

   if (mBatSpawnTimer >= 1.0f)
   {
      spawnBat();
      mBatSpawnTimer = 0.0f;
   }

   Vector2 playerPos = mGameState.player->getPosition();

   for (int i = 0; i < MAX_BATS; i++)
   {
      if (!mBats[i].active) continue;

      Bat *bat = &mBats[i];

      for (int w = 0; w < mGameState.activeWeaponCount; w++)
      {
         if (weaponHitsBat(&mGameState.weapon[w], bat))
         {
            if (bat->state != BAT_DEAD)
            {
               bat->state = BAT_DEAD;
               bat->currentFrame = 0;
               bat->animationTime = 0.0f;
               bat->frameSpeed = 0.08f;
            }
         }
      }

      if (bat->state == BAT_DEAD)
      {
         bat->animationTime += deltaTime;

         if (bat->animationTime >= bat->frameSpeed)
         {
            bat->animationTime = 0.0f;
            bat->currentFrame++;
         }

         if (bat->currentFrame >= BAT_DEATH_FRAMES)
         {
            bat->active = false;
            mBatKillCount++;
         }

         continue;
      }

      float distanceToPlayer = Vector2Distance(bat->position, playerPos);

      if (playerPos.x < bat->position.x) bat->facingLeft = true;
      else                               bat->facingLeft = false;

      if (distanceToPlayer < BAT_ATTACK_DISTANCE)
      {
         if (bat->state != BAT_ATTACKING)
         {
            bat->state = BAT_ATTACKING;
            bat->currentFrame = 0;
            bat->animationTime = 0.0f;
            bat->frameSpeed = 0.08f;
            bat->hasDamagedPlayer = false;
         }

         bat->velocity = { 0.0f, 0.0f };
      }
      else
      {
         bat->state = BAT_FLYING;

         Vector2 direction = {
            playerPos.x - bat->position.x,
            playerPos.y - bat->position.y
         };

         float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

         if (length > 0.0f)
         {
            direction.x /= length;
            direction.y /= length;
         }

         bat->velocity = {
            direction.x * BAT_SPEED,
            direction.y * BAT_SPEED
         };

         Vector2 nextPosition = {
            bat->position.x + bat->velocity.x * deltaTime,
            bat->position.y + bat->velocity.y * deltaTime
         };

         if (isBatBlockedAtPosition(bat, nextPosition))
         {
            bat->velocity = { 0.0f, 0.0f };
         }
         else
         {
            bat->position = nextPosition;
         }
      }
   

      int frameCount = BAT_FLY_FRAMES;
      if (bat->state == BAT_ATTACKING) frameCount = BAT_ATTACK_FRAMES;

      bat->animationTime += deltaTime;
      if (bat->animationTime >= bat->frameSpeed)
      {
         bat->animationTime = 0.0f;
         bat->currentFrame++;

         if (bat->state == BAT_ATTACKING)
         {
            if (bat->currentFrame >= frameCount)
            {
               if (!bat->hasDamagedPlayer)
               {
                  gLives--;
                  bat->hasDamagedPlayer = true;

                  if (gLives <= 0)
                  {
                     gBeatBoss = false;
                     mGameState.nextSceneID = 6;
                  }
               }

               bat->state = BAT_FLYING;
               bat->currentFrame = 0;
               bat->animationTime = 0.0f;
               bat->frameSpeed = 0.08f;
            }
         }
         else
         {
            if (bat->currentFrame >= frameCount)
               bat->currentFrame = 0;
         }
      }
   }

   if (mBatKillCount >= mBatKillTarget)
   {
      mGameState.nextSceneID = 3;
   }
}

void Level2::renderBat(Bat *bat)
{
   if (!bat->active) return;

   Texture2D texture;
   int frameCount = BAT_FLY_FRAMES;

   if (bat->state == BAT_FLYING)
   {
      texture = mBatFlyTexture;
      frameCount = BAT_FLY_FRAMES;
   }
   else if (bat->state == BAT_ATTACKING)
   {
      texture = mBatAttackTexture;
      frameCount = BAT_ATTACK_FRAMES;
   }
   else
   {
      texture = mBatDeathTexture;
      frameCount = BAT_DEATH_FRAMES;
   }

   float frameWidth  = (float)texture.width / frameCount;
   float frameHeight;

   Rectangle sourceArea;

   if (bat->state == BAT_ATTACKING)
   {
      frameHeight = (float)texture.height / 2.0f;

      int row = bat->facingLeft ? 0 : 1;

      sourceArea = {
         frameWidth * bat->currentFrame,
         frameHeight * row,
         frameWidth,
         frameHeight
      };
   }
   else
   {
      frameHeight = (float)texture.height;

      sourceArea = {
         frameWidth * bat->currentFrame,
         0.0f,
         frameWidth,
         frameHeight
      };
   }

   Rectangle destinationArea = {
      bat->position.x,
      bat->position.y,
      bat->scale.x,
      bat->scale.y
   };

   DrawTexturePro(
      texture,
      sourceArea,
      destinationArea,
      { bat->scale.x / 2.0f, bat->scale.y / 2.0f },
      0.0f,
      WHITE
   );
}

void Level2::update(float deltaTime)
{
   mGameState.player->update(
      deltaTime,
      nullptr,
      mGameState.map,
      nullptr,
      0
   );

   updateWeapons(deltaTime);
   updateBats(deltaTime);

   if (mGameState.player->getPosition().y > 800.0f)
   {
      gLives--;

      if (gLives <= 0)
      {
         gBeatBoss = false;
         mGameState.nextSceneID = 6;
      }
      else             mGameState.nextSceneID = 2;
   }
}

void Level2::render()
{
   mGameState.map->render();

   for (int i = 0; i < MAX_BATS; i++)
      renderBat(&mBats[i]);

   for (int i = 0; i < mGameState.activeWeaponCount; i++)
      renderWeapon(&mGameState.weapon[i]);

   mGameState.player->render();
}

void Level2::renderUI()
{
   DrawText("LEVEL 2", 20, 20, 30, WHITE);

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

   DrawText(TextFormat("Kills: %d / %d", mBatKillCount, mBatKillTarget),
            20, 90, 25, WHITE);
}

void Level2::shutdown()
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

   UnloadTexture(mBatFlyTexture);
   UnloadTexture(mBatAttackTexture);
   UnloadTexture(mBatDeathTexture);
   UnloadTexture(mHeartTexture);
}