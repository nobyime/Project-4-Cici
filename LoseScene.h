#ifndef LOSESCENE_H
#define LOSESCENE_H

#include "CS3113/Scene.h"

class LoseScene : public Scene {
public:
    LoseScene();
    LoseScene(Vector2 origin, const char *bgHexCode);
    ~LoseScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif