#pragma once
#include <mutex>

#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"
#include "ShaderWatcher.h"

class Engine {

public :
    Engine(Engine &other) = delete;

    void operator=(const Engine &) = delete;

    static Engine* GetInstance();

    AssetManager assetManager = {};
    GameScene scene = {};
    ShaderWatcher shaderWatcher = {};
    ProjectSettings settings = {};


    double runningTime;
    u64 frameCount;

    void UpdateSettings();



private:
    static Engine* self;

    static std::mutex _mutex;

    Engine() = default;

    ~Engine() = default;

};

