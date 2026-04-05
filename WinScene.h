#ifndef WINSCENE_H
#define WINSCENE_H

#include "CS3113/Scene.h"

class WinScene : public Scene {
public:
    WinScene();
    WinScene(Vector2 origin, const char *bgHexCode);
    ~WinScene();

    void initialise() override;
    void update(float deltaTime) override;
    void render() override;
    void shutdown() override;
};

#endif