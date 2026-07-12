#include "ShaderWatcher.h"
#include <iostream>

ShaderWatcher::ShaderWatcher() {
    workerThread = std::jthread([this](std::stop_token stToken) { WatcherThreadLoop(stToken); });
}

void ShaderWatcher::RegisterShader(Shader& shader) {
    std::string vertPath {};
    std::string frgmPath {};
    Shader::GetVertexFragmentPath(shader.GetName().c_str(), vertPath, frgmPath);

    auto tv = GetFileTime(vertPath);
    auto tf = GetFileTime(vertPath);

    TrackedShader ts{shader.GetName(), tv, tf};

    std::lock_guard lock(queueMutex);
    trackedShaders.push_back(ts);
}

void ShaderWatcher::ProcessReloads() {
    std::vector<TrackedShader> toReload;
    {
        std::lock_guard lock(queueMutex);
        toReload = reloadQueue;
        reloadQueue.clear();
    }
    for (const auto& h : toReload) {
        std::cout << "[ShaderWatcher] Recompiling shader:" << h.shaderName << "\n";
    }
}


fs::file_time_type ShaderWatcher::GetFileTime(const std::string& path) {
    std::error_code ec;
    auto time = fs::last_write_time(path, ec);
    return ec ? (fs::file_time_type::min)(): time;
}

void ShaderWatcher::WatcherThreadLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        // sleep for .5 secons
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // scoped lock
        std::lock_guard lock(queueMutex);

        for (auto& shader : trackedShaders) {

            std::string vertPath {};
            std::string frgmPath {};
            Shader::GetVertexFragmentPath(shader.shaderName.c_str(), vertPath, frgmPath);

            auto newVertTime = GetFileTime(vertPath);
            auto newFragTime = GetFileTime(frgmPath);
            bool changed = false;
            if (newVertTime > shader.vertTime) {
                shader.vertTime = newVertTime;
                changed = true;
            }
            if (newFragTime > shader.fragTime) {
                shader.fragTime = newFragTime;
                changed = true;
            }
            if (changed) {
                reloadQueue.push_back(shader);
                std::cout << "Change detected !!!!" << shader.shaderName << std::endl;
            }
        }
    }
}


