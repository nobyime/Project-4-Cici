#include "Level3.h"

extern int gLives;
extern bool gBeatBoss;

Level3::Level3()                                      : Scene { {0.0f}, nullptr   } {}
Level3::Level3(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}
Level3::~Level3() { }

void Level3::initialiseWeapons()
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

void Level3::renderWeapon(Weapon *weapon)
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

void Level3::updateWeapons(float deltaTime)
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

void Level3::initialiseSlimes()
{
   for (int i = 0; i < MAX_SLIMES; i++)
   {
      mSlimes[i].position = { 0.0f, 0.0f };
      mSlimes[i].velocity = { 0.0f, 0.0f };
      mSlimes[i].scale = { SLIME_SCALE, SLIME_SCALE };

      mSlimes[i].currentFrame = 0;
      mSlimes[i].animationTime = 0.0f;
      mSlimes[i].frameSpeed = 0.08f;

      mSlimes[i].active = false;
      mSlimes[i].facingLeft = true;
      mSlimes[i].hasDamagedPlayer = false;
      mSlimes[i].diedByHittingPlayer = false;
      mSlimes[i].state = SLIME_WALKING;

      mSlimes[i].walkTimer = 0.0f;
      mSlimes[i].walkDuration = 0.0f;
      mSlimes[i].jumpInitialised = false;
      mSlimes[i].jumpStart = { 0.0f, 0.0f };
      mSlimes[i].jumpEnd = { 0.0f, 0.0f };
      mSlimes[i].jumpTimer = 0.0f;
   }
}

void Level3::initialise()
{
   mGameState.nextSceneID = -1;
   mGameState.facingLeft = false;

   mGameState.jumpSound = LoadSound("bgm/jump_on_grass.wav");
   SetSoundVolume(mGameState.jumpSound, 0.3f);

   mGameState.walkSound = LoadSound("bgm/walk_on_grass.wav");
   SetSoundVolume(mGameState.walkSound, 1.5f);

   mGameState.map = new Map(
      LEVEL3_WIDTH, LEVEL3_HEIGHT,
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

   mSlimeWalkTexture  = LoadTexture("images/slime_walk.png");
   mSlimeJumpTexture  = LoadTexture("images/slime_jump.png");
   mSlimeDeathTexture = LoadTexture("images/slime_death.png");
   mHeartTexture      = LoadTexture("images/heart.png");

   initialiseSlimes();

   mSlimeSpawnTimer = 0.0f;
   mSlimeKillCount = 0;
   mSlimeKillTarget = 30;
}

bool Level3::weaponHitsSlime(Weapon *weapon, Slime *slime)
{
   float dx = fabs(weapon->position.x - slime->position.x);
   float dy = fabs(weapon->position.y - slime->position.y);

   return (dx < 38.0f && dy < 38.0f);
}

bool Level3::isGroundAtPoint(Vector2 point)
{
   float xOverlap = 0.0f;
   float yOverlap = 0.0f;
   return mGameState.map->isSolidTileAt(point, &xOverlap, &yOverlap);
}

float Level3::getGroundYAtX(float x)
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

void Level3::chooseNextSlimeAction(Slime *slime)
{
   int action = GetRandomValue(0, 1);

   if (action == 0)
   {
      slime->state = SLIME_JUMPING;
      slime->currentFrame = 0;
      slime->animationTime = 0.0f;
      slime->frameSpeed = 0.1f;
      slime->jumpTimer = 0.0f;
      slime->jumpInitialised = false;
   }
   else
   {
      slime->state = SLIME_WALKING;
      slime->currentFrame = 0;
      slime->animationTime = 0.0f;
      slime->frameSpeed = 0.08f;
      slime->walkTimer = 0.0f;
      slime->walkDuration = (float)GetRandomValue(6, 12) / 10.0f;
   }
}

void Level3::spawnSlime()
{
   for (int i = 0; i < MAX_SLIMES; i++)
   {
      if (!mSlimes[i].active)
      {
         mSlimes[i].active = true;
         mSlimes[i].currentFrame = 0;
         mSlimes[i].animationTime = 0.0f;
         mSlimes[i].frameSpeed = 0.08f;
         mSlimes[i].hasDamagedPlayer = false;
         mSlimes[i].diedByHittingPlayer = false;
         mSlimes[i].jumpInitialised = false;
         mSlimes[i].jumpTimer = 0.0f;

         bool spawnLeft = GetRandomValue(0, 1) == 0;

         if (spawnLeft)
         {
            mSlimes[i].position = {
               mOrigin.x + SLIME_LEFT_SPAWN_X,
               mOrigin.y - 120.0f
            };
            mSlimes[i].facingLeft = false;
         }
         else
         {
            mSlimes[i].position = {
               mOrigin.x + SLIME_RIGHT_SPAWN_X,
               mOrigin.y - 120.0f
            };
            mSlimes[i].facingLeft = true;
         }

         mSlimes[i].state = SLIME_JUMPING;
         mSlimes[i].walkTimer = 0.0f;
         mSlimes[i].walkDuration = 0.0f;

         break;
      }
   }
}

Slime* Level3::findNearestTargetOnFacingSide()
{
   Slime *nearest = nullptr;
   float bestDistance = 999999.0f;

   Vector2 playerPos = mGameState.player->getPosition();
   bool facingLeft = mGameState.facingLeft;

   for (int i = 0; i < MAX_SLIMES; i++)
   {
      if (!mSlimes[i].active) continue;
      if (mSlimes[i].state == SLIME_DYING) continue;

      if (facingLeft && mSlimes[i].position.x >= playerPos.x) continue;
      if (!facingLeft && mSlimes[i].position.x <= playerPos.x) continue;

      float distance = Vector2Distance(playerPos, mSlimes[i].position);

      if (distance < bestDistance)
      {
         bestDistance = distance;
         nearest = &mSlimes[i];
      }
   }

   return nearest;
}

void Level3::throwWeaponAtNearestSlime()
{
   Slime *target = findNearestTargetOnFacingSide();
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

void Level3::updateSlimes(float deltaTime)
{
   mSlimeSpawnTimer += deltaTime;

   if (mSlimeSpawnTimer >= 1.0f)
   {
      spawnSlime();
      mSlimeSpawnTimer = 0.0f;
   }

   Vector2 playerPos = mGameState.player->getPosition();

   for (int i = 0; i < MAX_SLIMES; i++)
   {
      if (!mSlimes[i].active) continue;

      Slime *slime = &mSlimes[i];

      for (int w = 0; w < mGameState.activeWeaponCount; w++)
      {
         if (weaponHitsSlime(&mGameState.weapon[w], slime))
         {
            if (slime->state != SLIME_DYING)
            {
               slime->state = SLIME_DYING;
               slime->currentFrame = 0;
               slime->animationTime = 0.0f;
               slime->frameSpeed = 0.08f;
               slime->diedByHittingPlayer = false;
            }
         }
      }

      if (slime->state != SLIME_DYING)
      {
         float dx = fabs(slime->position.x - playerPos.x);
         float dy = fabs(slime->position.y - playerPos.y);

         if (dx < 40.0f && dy < 40.0f)
         {
            slime->state = SLIME_DYING;
            slime->currentFrame = 0;
            slime->animationTime = 0.0f;
            slime->frameSpeed = 0.08f;
            slime->diedByHittingPlayer = true;
            slime->hasDamagedPlayer = false;
         }
      }

      if (slime->state == SLIME_DYING)
      {
         slime->animationTime += deltaTime;

         if (slime->animationTime >= slime->frameSpeed)
         {
            slime->animationTime = 0.0f;
            slime->currentFrame++;
         }

         if (slime->currentFrame >= SLIME_DEATH_FRAMES)
         {
            if (slime->diedByHittingPlayer && !slime->hasDamagedPlayer)
            {
               gLives--;
               slime->hasDamagedPlayer = true;

               if (gLives <= 0)
               {
                  gBeatBoss = false;
                  mGameState.nextSceneID = 6;
               }
            }

            slime->active = false;

            if (!slime->diedByHittingPlayer)
               mSlimeKillCount++;
         }

         continue;
      }

      if (slime->state == SLIME_WALKING)
      {
         slime->walkTimer += deltaTime;

         if (playerPos.x < slime->position.x)
         {
            slime->velocity.x = -SLIME_MOVE_SPEED;
            slime->facingLeft = true;
         }
         else
         {
            slime->velocity.x = SLIME_MOVE_SPEED;
            slime->facingLeft = false;
         }

         slime->position.x += slime->velocity.x * deltaTime;

         float footY = slime->position.y + slime->scale.y * 0.15f;
         Vector2 bottomProbe = { slime->position.x, footY };

         float xOverlap = 0.0f;
         float yOverlap = 0.0f;

         if (mGameState.map->isSolidTileAt(bottomProbe, &xOverlap, &yOverlap))
         {
            slime->position.y -= yOverlap;
         }

         if (slime->walkTimer >= slime->walkDuration)
         {
            chooseNextSlimeAction(slime);
         }
      }
      else if (slime->state == SLIME_JUMPING)
      {
         if (!slime->jumpInitialised)
         {
            slime->jumpStart = slime->position;

            if (playerPos.x < slime->position.x)
            {
               slime->jumpEnd.x = slime->position.x - SLIME_JUMP_DISTANCE;
               slime->facingLeft = true;
            }
            else
            {
               slime->jumpEnd.x = slime->position.x + SLIME_JUMP_DISTANCE;
               slime->facingLeft = false;
            }

            slime->jumpEnd.y = getGroundYAtX(slime->jumpEnd.x);
            slime->jumpTimer = 0.0f;
            slime->jumpInitialised = true;
         }

         slime->jumpTimer += deltaTime;
         float t = slime->jumpTimer / SLIME_JUMP_DURATION;
         if (t > 1.0f) t = 1.0f;

         slime->position.x =
            slime->jumpStart.x + (slime->jumpEnd.x - slime->jumpStart.x) * t;

         float baseY =
            slime->jumpStart.y + (slime->jumpEnd.y - slime->jumpStart.y) * t;

         float arc = 4.0f * SLIME_JUMP_HEIGHT * t * (1.0f - t);

         slime->position.y = baseY - arc;

         if (t >= 1.0f)
         {
            slime->position = slime->jumpEnd;
            chooseNextSlimeAction(slime);
         }
      }

      int frameCount = SLIME_WALK_FRAMES;
      if (slime->state == SLIME_JUMPING) frameCount = SLIME_JUMP_FRAMES;

      slime->animationTime += deltaTime;
      if (slime->animationTime >= slime->frameSpeed)
      {
         slime->animationTime = 0.0f;
         slime->currentFrame++;

         if (slime->currentFrame >= frameCount)
            slime->currentFrame = 0;
      }
   }

   if (mSlimeKillCount >= mSlimeKillTarget)
   {
      mGameState.nextSceneID = 5;
   }
}

void Level3::renderSlime(Slime *slime)
{
   if (!slime->active) return;

   Texture2D texture;
   int frameCount;

   if (slime->state == SLIME_WALKING)
   {
      texture = mSlimeWalkTexture;
      frameCount = SLIME_WALK_FRAMES;
   }
   else if (slime->state == SLIME_JUMPING)
   {
      texture = mSlimeJumpTexture;
      frameCount = SLIME_JUMP_FRAMES;
   }
   else
   {
      texture = mSlimeDeathTexture;
      frameCount = SLIME_DEATH_FRAMES;
   }

   float frameWidth  = (float)texture.width / frameCount;
   float frameHeight = (float)texture.height / 2.0f;

   int row = slime->facingLeft ? 1 : 0;

   Rectangle sourceArea = {
      frameWidth * slime->currentFrame,
      frameHeight * row,
      frameWidth,
      frameHeight
   };

   Rectangle destinationArea = {
      slime->position.x,
      slime->position.y,
      slime->scale.x,
      slime->scale.y
   };

   DrawTexturePro(
      texture,
      sourceArea,
      destinationArea,
      { slime->scale.x / 2.0f, slime->scale.y / 2.0f },
      0.0f,
      WHITE
   );
}

void Level3::update(float deltaTime)
{
   mGameState.player->update(
      deltaTime,
      nullptr,
      mGameState.map,
      nullptr,
      0
   );

   updateWeapons(deltaTime);
   updateSlimes(deltaTime);

   if (mGameState.player->getPosition().y > 800.0f)
   {
      gLives--;

      if (gLives <= 0)
      {
         gBeatBoss = false;
         mGameState.nextSceneID = 6;
      }
      else             mGameState.nextSceneID = 3;
   }
}

void Level3::render()
{
   mGameState.map->render();

   for (int i = 0; i < MAX_SLIMES; i++)
      renderSlime(&mSlimes[i]);

   for (int i = 0; i < mGameState.activeWeaponCount; i++)
      renderWeapon(&mGameState.weapon[i]);

   mGameState.player->render();
}

void Level3::renderUI()
{
   DrawText("LEVEL 3", 20, 20, 30, WHITE);

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

   DrawText(TextFormat("Kills: %d / %d", mSlimeKillCount, mSlimeKillTarget),
            20, 90, 25, WHITE);
}

void Level3::shutdown()
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

   UnloadTexture(mSlimeWalkTexture);
   UnloadTexture(mSlimeJumpTexture);
   UnloadTexture(mSlimeDeathTexture);
   UnloadTexture(mHeartTexture);
}