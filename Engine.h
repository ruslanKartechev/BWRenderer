#pragma once
#include <mutex>
#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"
#include "ShaderWatcher.h"
#include "myGui.h"

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
    //endregion

    // core Metrics
    double runningTime;
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

