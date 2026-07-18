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
    std::string name;
    std::string pathV;
    std::string pathF;

    fs::file_time_type vertTime;
    fs::file_time_type fragTime;
};


struct TrackedFile {
    std::string filePath;
    fs::file_time_type lastChangeTime;
    bool updated;
};



class ShaderWatcher {
public:
    ShaderWatcher();

    void WatchShader(Shader& shader);

    void WatchSettingsFile();

    void ProcessReloads();

private:
    std::vector<TrackedShader> trackedShaders;
    std::vector<TrackedShader> reloadQueue;
    std::mutex queueMutex;
    std::jthread workerThread;
    TrackedFile trackedSettings;

    static fs::file_time_type GetFileTime(const std::string& path);

    void WatcherThreadLoop(std::stop_token stopToken);

};