#include "ShaderWatcher.h"
#include <iostream>
#include "Engine.h"
#include "ProjectDefines.h"

ShaderWatcher::ShaderWatcher() {
    workerThread = std::jthread([this](std::stop_token stToken) { WatcherThreadLoop(stToken); });
}

fs::file_time_type ShaderWatcher::GetFileTime(const std::string& path) {
    std::error_code ec;
    fs::file_time_type time = fs::last_write_time(path, ec);
    return ec ? (fs::file_time_type::min)(): time;
}


void ShaderWatcher::WatchSettingsFile() {
    auto path = AssetManager::GetProjectSettingsPath();
    trackedSettings.filePath = std::string(path);

    trackedSettings.lastChangeTime = GetFileTime(path);
    trackedSettings.updated = false;
}

void ShaderWatcher::WatchShader(Shader& shader) {
    std::string vertPath {};
    std::string frgmPath {};
    Shader::GetVertexFragmentPath(shader.GetName().c_str(), vertPath, frgmPath);

    auto tv = GetFileTime(vertPath);
    auto tf = GetFileTime(vertPath);

    TrackedShader ts{shader.GetName(), tv, tf};

    std::lock_guard lock(queueMutex);
    trackedShaders.push_back(ts);
}

/// Executes on the main thread
void ShaderWatcher::ProcessReloads() {
    std::vector<TrackedShader> toReload;
    {
        std::lock_guard lock(queueMutex);
        toReload = reloadQueue;
        reloadQueue.clear();
    }
    Engine& engine = *(Engine::GetInstance());

    for (const auto& h : toReload) {
#ifdef LOG_SHADER_WATCHER
        std::cout << "[ShaderWatcher] Recompiling shader:" << h.shaderName << "\n";
#endif
        auto handle = engine.assetManager.FindShaderByName(h.shaderName.c_str());
        if (handle.IsEmpty()) {
            std::cerr << "failed to find shader: " << h.shaderName << std::endl;
            continue;
        }
        Shader& shader = engine.assetManager.shaders.GetItemRef(handle);
        shader.Recompile();
    }

    if (trackedSettings.updated) {
        trackedSettings.updated = false;
        engine.UpdateSettings();
    }


}


/// Executes on a background thread
void ShaderWatcher::WatcherThreadLoop(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        // sleep for 0.5 second
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // scoped lock
        std::lock_guard lock(queueMutex);

        // Check project settings file if it exists
        if (trackedSettings.filePath.empty() == false) {
            auto newTime = GetFileTime(trackedSettings.filePath);

            if (newTime > trackedSettings.lastChangeTime) {
                trackedSettings.lastChangeTime = newTime;
                trackedSettings.updated = true;
            }
        }

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
#ifdef LOG_SHADER_WATCHER
                std::cout << "Change detected !!!!" << shader.shaderName << std::endl;
#endif
            }
        }
    }
}


