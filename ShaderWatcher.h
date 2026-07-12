#pragma once

#include <filesystem>
#include <thread>
#include <mutex>
#include <vector>
#include <string>
#include <chrono>
#include "Shader.h"


namespace fs = std::filesystem;

struct TrackedShader {
    std::string shaderName;
    fs::file_time_type vertTime;
    fs::file_time_type fragTime;
};


class ShaderWatcher {
public:
    ShaderWatcher();

    void RegisterShader(Shader& shader);

    void ProcessReloads();

private:
    std::vector<TrackedShader> trackedShaders;
    std::vector<TrackedShader> reloadQueue;
    std::mutex queueMutex;
    std::jthread workerThread;

    static fs::file_time_type GetFileTime(const std::string& path);

    void WatcherThreadLoop(std::stop_token stopToken);

};