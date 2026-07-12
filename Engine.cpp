#include "Engine.h"

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