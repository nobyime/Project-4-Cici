#include "Level1.h"

extern int gLives;
extern bool gBeatBoss;

Level1::Level1()                                      : Scene { {0.0f}, nullptr   } {}
Level1::Level1(Vector2 origin, const char *bgHexCode) : Scene { origin, bgHexCode } {}

Level1::~Level1() { }

void Level1::initialiseWeapons()
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

void Level1::initialiseCrawlers()
{
   for (int i = 0; i < MAX_CRAWLERS; i++)
   {
      mCrawlers[i].position = { 0.0f, 0.0f };
      mCrawlers[i].velocity = { 0.0f, 0.0f };
      mCrawlers[i].scale = { 100.0f, 100.0f };

      mCrawlers[i].currentFrame = 0;
      mCrawlers[i].animationTime = 0.0f;
      mCrawlers[i].frameSpeed = 0.08f;

      mCrawlers[i].active = false;
      mCrawlers[i].facingLeft = true;
      mCrawlers[i].state = CRAWLER_WALKING;
   }
}

void Level1::initialise()
{
   mGameState.nextSceneID = -1;
   mGameState.facingLeft = false;

   mGameState.jumpSound = LoadSound("bgm/jump_on_grass.wav");
   SetSoundVolume(mGameState.jumpSound, 0.3f);

   mGameState.walkSound = LoadSound("bgm/walk_on_grass.wav");
   SetSoundVolume(mGameState.walkSound, 1.5f);

   /*
      ----------- MAP -----------
   */
   mGameState.map = new Map(
      LEVEL1_WIDTH, LEVEL1_HEIGHT,   // map grid cols & rows
      (unsigned int *) mLevelData, // grid data
      "images/tile.png",           // texture filepath
      TILE_DIMENSION,              // tile size
      7, 1,                        // texture cols & rows
      mOrigin                      // in-game origin
   );

   /*
      ----------- PROTAGONIST -----------
   */
   std::map<Direction, std::vector<int>> playerAnimationAtlas = {
      {LEFT,  {0, 1, 2, 3, 4, 5, 6, 7, 8}},
      {RIGHT, {9, 10, 11, 12, 13, 14, 15, 16, 17}}
   };

   mGameState.player = new Entity(
      {mOrigin.x - 450.0f, mOrigin.y - 150.0f}, // position
      {100.0f, 100.0f},                         // scale
      "images/me.png",                          // texture file address
      ATLAS,                                    // single image or atlas?
      { 9, 2 },                                 // atlas dimensions
      playerAnimationAtlas,                    // actual atlas
      PLAYER                                    // entity type
   );

   mGameState.player->setJumpingPower(550.0f);
   mGameState.player->setColliderDimensions({
      mGameState.player->getScale().x / 3.5f,
      mGameState.player->getScale().y / 3.0f
   });
   mGameState.player->setAcceleration({0.0f, ACCELERATION_OF_GRAVITY});

   mGameState.activeWeaponCount = 1;

   initialiseWeapons();
   mCrawlerWalkTexture   = LoadTexture("images/crawler_walk.png");
   mCrawlerAttackTexture = LoadTexture("images/crawler_attack.png");
   mCrawlerDeathTexture  = LoadTexture("images/crawler_death.png");
   mHeartTexture = LoadTexture("images/heart.png");
   initialiseCrawlers();

   mCrawlerSpawnTimer = 0.0f;
   mCrawlerKillCount = 0;
   mCrawlerKillTarget = 20;
}

bool Level1::weaponHitsCrawler(Weapon *weapon, Crawler *crawler)
{
   float dx = fabs(weapon->position.x - crawler->position.x);
   float dy = fabs(weapon->position.y - crawler->position.y);

   return (dx < 45.0f && dy < 45.0f);
}

void Level1::spawnCrawler()
{
   for (int i = 0; i < MAX_CRAWLERS; i++)
   {
      if (!mCrawlers[i].active)
      {
         mCrawlers[i].active = true;
         mCrawlers[i].state = CRAWLER_WALKING;
         mCrawlers[i].currentFrame = 0;
         mCrawlers[i].animationTime = 0.0f;
         mCrawlers[i].frameSpeed = 0.08f;
         mCrawlers[i].hasDamagedPlayer = false;

         bool spawnLeft = GetRandomValue(0, 1) == 0;

         if (spawnLeft)
         {
            mCrawlers[i].position = {
               mOrigin.x + CRAWLER_LEFT_SPAWN_X,
               mOrigin.y + CRAWLER_SPAWN_Y
            };
            mCrawlers[i].facingLeft = false;
         }
         else
         {
            mCrawlers[i].position = {
               mOrigin.x + CRAWLER_RIGHT_SPAWN_X,
               mOrigin.y + CRAWLER_SPAWN_Y
            };
            mCrawlers[i].facingLeft = true;
         }

         break;
      }
   }
}

void Level1::updateWeapons(float deltaTime)
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

void Level1::renderWeapon(Weapon *weapon)
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

void Level1::updateCrawlers(float deltaTime)
{
   mCrawlerSpawnTimer += deltaTime;

   if (mCrawlerSpawnTimer >= 1.0f)
   {
      spawnCrawler();
      mCrawlerSpawnTimer = 0.0f;
   }

   Vector2 playerPos = mGameState.player->getPosition();

   for (int i = 0; i < MAX_CRAWLERS; i++)
   {
      if (!mCrawlers[i].active) continue;

      Crawler *crawler = &mCrawlers[i];

      for (int w = 0; w < mGameState.activeWeaponCount; w++)
      {
         if (weaponHitsCrawler(&mGameState.weapon[w], crawler))
         {
            if (crawler->state != CRAWLER_DEAD)
            {
               crawler->state = CRAWLER_DEAD;
               crawler->currentFrame = 0;
               crawler->animationTime = 0.0f;
               crawler->frameSpeed = 0.08f;
            }
         }
      }

      if (crawler->state == CRAWLER_DEAD)
      {
         crawler->animationTime += deltaTime;

         if (crawler->animationTime >= crawler->frameSpeed)
         {
            crawler->animationTime = 0.0f;
            crawler->currentFrame++;
         }

         if (crawler->currentFrame >= 8)
         {
            crawler->active = false;
            mCrawlerKillCount++;
         }

         continue;
      }

      crawler->velocity.y += ACCELERATION_OF_GRAVITY * deltaTime;
      crawler->position.y += crawler->velocity.y * deltaTime;

      float xOverlap = 0.0f;
      float yOverlap = 0.0f;

      Vector2 bottomProbe = {
         crawler->position.x,
         crawler->position.y + crawler->scale.y * 0.15f
      };

      bool onGround = false;

      if (mGameState.map->isSolidTileAt(bottomProbe, &xOverlap, &yOverlap))
      {
         crawler->position.y -= yOverlap;
         crawler->velocity.y = 0.0f;
         onGround = true;
      }

      if (!onGround)
      {
         crawler->velocity.x = 0.0f;
         crawler->state = CRAWLER_WALKING;
         crawler->currentFrame = 0;
         crawler->animationTime = 0.0f;
         continue;
      }

      float distanceToPlayer = Vector2Distance(crawler->position, playerPos);

      if (distanceToPlayer < 50.0f)
      {
         if (crawler->state != CRAWLER_ATTACKING)
         {
            crawler->state = CRAWLER_ATTACKING;
            crawler->currentFrame = 0;
            crawler->animationTime = 0.0f;
            crawler->frameSpeed = 0.08f;
            crawler->hasDamagedPlayer = false;
         }

         crawler->velocity.x = 0.0f;

         if (playerPos.x < crawler->position.x) crawler->facingLeft = true;
         else                                   crawler->facingLeft = false;
      }
      else
      {
         if (crawler->state != CRAWLER_WALKING)
         {
            crawler->state = CRAWLER_WALKING;
            crawler->currentFrame = 0;
            crawler->animationTime = 0.0f;
            crawler->frameSpeed = 0.08f;
         }

         if (playerPos.x < crawler->position.x)
         {
            crawler->velocity.x = -80.0f;
            crawler->facingLeft = true;
         }
         else
         {
            crawler->velocity.x = 80.0f;
            crawler->facingLeft = false;
         }
      }

      crawler->position.x += crawler->velocity.x * deltaTime;

      float stepHeight = 30.0f;
      float frontOffset = crawler->facingLeft ? -crawler->scale.x / 4.0f : crawler->scale.x / 4.0f;

      Vector2 frontLowProbe = {
         crawler->position.x + frontOffset,
         crawler->position.y + crawler->scale.y / 2.0f
      };

      Vector2 frontHighProbe = {
         crawler->position.x + frontOffset,
         crawler->position.y + crawler->scale.y / 2.0f - stepHeight
      };

      float lowXOverlap = 0.0f, lowYOverlap = 0.0f;
      float highXOverlap = 0.0f, highYOverlap = 0.0f;

      bool frontLowBlocked  = mGameState.map->isSolidTileAt(frontLowProbe, &lowXOverlap, &lowYOverlap);
      bool frontHighBlocked = mGameState.map->isSolidTileAt(frontHighProbe, &highXOverlap, &highYOverlap);

      if (frontLowBlocked && !frontHighBlocked)
      {
         crawler->position.y -= stepHeight;
      }

      if (crawler->position.y > mOrigin.y + 900.0f)
      {
         crawler->active = false;
      }

      int frameCount = 5;
      if (crawler->state == CRAWLER_ATTACKING) frameCount = 14;

      crawler->animationTime += deltaTime;
      if (crawler->animationTime >= crawler->frameSpeed)
      {
         crawler->animationTime = 0.0f;
         crawler->currentFrame++;

         if (crawler->state == CRAWLER_ATTACKING)
         {
            if (crawler->currentFrame >= frameCount)
            {
               if (!crawler->hasDamagedPlayer)
               {
                  gLives--;
                  crawler->hasDamagedPlayer = true;

                  float pushDistance = 180.0f;
                  crawler->velocity.x = 0.0f;

                  if (crawler->position.x < mGameState.player->getPosition().x)
                     crawler->position.x -= pushDistance;
                  else
                     crawler->position.x += pushDistance;

                  if (gLives <= 0)
                  {
                     gBeatBoss = false;
                     mGameState.nextSceneID = 6;
                  }
               }

               crawler->state = CRAWLER_WALKING;
               crawler->currentFrame = 0;
               crawler->animationTime = 0.0f;
               crawler->frameSpeed = 0.08f;
            }
         }
         else
         {
            if (crawler->currentFrame >= frameCount)
               crawler->currentFrame = 0;
         }
      }
   }

   if (mCrawlerKillCount >= mCrawlerKillTarget)
   {
      mGameState.nextSceneID = 2;
   }
}

void Level1::update(float deltaTime)
{
   mGameState.player->update(
      deltaTime,      // delta time / fixed timestep
      nullptr,        // player
      mGameState.map, // map
      nullptr,        // collidable entities
      0               // col. entity count
   );

   updateWeapons(deltaTime);
   updateCrawlers(deltaTime);

   // fell off map
   if (mGameState.player->getPosition().y > 800.0f)
   {
      gLives--;

      if (gLives <= 0)
      {
         gBeatBoss = false;
         mGameState.nextSceneID = 6;
      }
      else             mGameState.nextSceneID = 1;
   }
}

void Level1::render()
{
   mGameState.map->render();

   for (int i = 0; i < MAX_CRAWLERS; i++)
      renderCrawler(&mCrawlers[i]);

   for (int i = 0; i < mGameState.activeWeaponCount; i++)
      renderWeapon(&mGameState.weapon[i]);

   mGameState.player->render();
}

/*
void Level1::renderCrawler(Crawler *crawler)
{
   if (!crawler->active) return;

   Texture2D texture;
   int frameCount = 5;

   if (crawler->state == CRAWLER_WALKING)
   {
      texture = mCrawlerWalkTexture;
      frameCount = 5;
   }
   else if (crawler->state == CRAWLER_ATTACKING)
   {
      texture = mCrawlerAttackTexture;
      frameCount = 14;
   }
   else
   {
      texture = mCrawlerDeathTexture;
      frameCount = 8;
   }

   float frameWidth  = (float)texture.width / frameCount;
   float frameHeight = (float)texture.height / 2.0f;

   int row = crawler->facingLeft ? 0 : 1;

   Rectangle sourceArea = {
      frameWidth * crawler->currentFrame,
      frameHeight * row,
      frameWidth,
      frameHeight
   };

   Rectangle destinationArea = {
      crawler->position.x,
      crawler->position.y,
      crawler->scale.x,
      crawler->scale.y
   };

   DrawTexturePro(
      texture,
      sourceArea,
      destinationArea,
      { crawler->scale.x / 2.0f, crawler->scale.y / 2.0f },
      0.0f,
      WHITE
   );
}
*/

void Level1::renderCrawler(Crawler *crawler)
{
   if (!crawler->active) return;

   Texture2D texture;
   int frameCount = 5;

   if (crawler->state == CRAWLER_WALKING)
   {
      texture = mCrawlerWalkTexture;
      frameCount = 5;
   }
   else if (crawler->state == CRAWLER_ATTACKING)
   {
      texture = mCrawlerAttackTexture;
      frameCount = 14;
   }
   else
   {
      texture = mCrawlerDeathTexture;
      frameCount = 8;
   }

   float frameWidth  = (float)texture.width / frameCount;
   float frameHeight = (float)texture.height / 2.0f;

   Rectangle sourceArea = {
      frameWidth * crawler->currentFrame,
      0.0f,
      frameWidth,
      frameHeight
   };

   if (!crawler->facingLeft)
   {
      sourceArea.x += frameWidth;
      sourceArea.width = -frameWidth;
   }

   Rectangle destinationArea = {
      crawler->position.x,
      crawler->position.y,
      crawler->scale.x,
      crawler->scale.y
   };

   DrawTexturePro(
      texture,
      sourceArea,
      destinationArea,
      { crawler->scale.x / 2.0f, crawler->scale.y / 2.0f },
      0.0f,
      WHITE
   );
}

//*

void Level1::renderUI()
{
   DrawText("LEVEL 1", 20, 20, 30, WHITE);

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

   DrawText(TextFormat("Kills: %d / %d", mCrawlerKillCount, mCrawlerKillTarget),
            20, 90, 25, WHITE);
}

void Level1::shutdown()
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

   UnloadTexture(mCrawlerWalkTexture);
   UnloadTexture(mCrawlerAttackTexture);
   UnloadTexture(mCrawlerDeathTexture);
   UnloadTexture(mHeartTexture);
}