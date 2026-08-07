#pragma once
#include <mutex>
#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"
#include "ShaderWatcher.h"
#include "myGui.h"
#include "Script.h"

class Engine {

public :
    Engine(Engine &other) = delete;

    void operator=(const Engine &) = delete;

    // region Core Components
    AssetManager assetManager = {};
    GameScene scene = {};
    ShaderWatcher shaderWatcher = {};
    ProjectSettings settings = {};
    MyGui gui = {};

    std::vector<Script> scripts = {};

    //endregion

    // core Metrics
    f64 runningTime;
    f32 deltaTime;
    u64 frameCount;

    // endregion

    static Engine* GetInstance();

    void UpdateSettings();


private:
    static Engine* self;

    static std::mutex _mutex;

    Engine() = default;

    ~Engine() = default;

};

