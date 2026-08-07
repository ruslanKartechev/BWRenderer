#pragma once
#include "../MyTypes.h"
#include "../Terrain.h"
#include "../Transform.h"

class AssetManager;

class ScriptTerrainGame{
public:
    void Create();
    void Start();
    void GamePause();
    void GameResume();
    void Quit();
    void Update(f32 deltaTime);
    void PhysicsUpdate(f32 deltaTime);
    void GUIUpdate(f32 deltaTime);

private:
    void RegenTerrain();
    void InitTreesInstanceBuffer();
    void PlaceTreesOnTerrain(Terrain& terrain, Transform& terrainTransform);
};
