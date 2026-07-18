#include "Engine.h"
#include <iostream>

Engine* Engine::self = nullptr;
std::mutex Engine::_mutex = {};

Engine* Engine::GetInstance() {
    std::lock_guard lock(_mutex);
    if (self == nullptr)
    {
        self = new Engine();
    }
    return self;
}

void Engine::UpdateSettings() {
    bool didParse = ProjectSettings::LoadProjectSettings(AssetManager::GetProjectSettingsPath(), settings);
    if (didParse) {
        std::cout << "[Engine] Parsed project settings file!" << std::endl;
    }
    else {
        std::cerr << "Failed to parse the config file ((" << std::endl;
    }
}