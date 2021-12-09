//*****************************************************************************
// Copyright 2021 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#include "custom_node_library_manager.hpp"

#include <utility>

#include <dlfcn.h>

#include "filesystem.hpp"
#include "logging.hpp"
#include "status.hpp"

namespace ovms {

Status CustomNodeLibraryManager::loadLibrary(const std::string& name, const std::string& basePath) {
    if (FileSystem::isPathEscaped(basePath)) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Path {} escape with .. is forbidden.", basePath);
        return StatusCode::PATH_INVALID;
    }

    auto it = libraries.find(name);
    if (it != libraries.end() && it->second.basePath == basePath) {
        SPDLOG_LOGGER_DEBUG(modelmanager_logger, "Custom node library name: {} is already loaded", name);
        return StatusCode::NODE_LIBRARY_ALREADY_LOADED;
    }

    SPDLOG_LOGGER_INFO(modelmanager_logger, "Loading custom node library name: {}; base_path: {}", name, basePath);

    void* handle = dlopen(basePath.c_str(), RTLD_LAZY | RTLD_LOCAL);
    char* error = dlerror();
    if (handle == NULL) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Library name: {} failed to open base_path: {} with error: {}", name, basePath, error);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_OPEN;
    }

    initialize_fn initialize = reinterpret_cast<initialize_fn>(dlsym(handle, "initialize"));
    error = dlerror();
    if (error || initialize == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    deinitialize_fn deinitialize = reinterpret_cast<deinitialize_fn>(dlsym(handle, "deinitialize"));
    error = dlerror();
    if (error || deinitialize == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    execute_fn execute = reinterpret_cast<execute_fn>(dlsym(handle, "execute"));
    error = dlerror();
    if (error || execute == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    metadata_fn getInputsInfo = reinterpret_cast<metadata_fn>(dlsym(handle, "getInputsInfo"));
    error = dlerror();
    if (error || getInputsInfo == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    metadata_fn getOutputsInfo = reinterpret_cast<metadata_fn>(dlsym(handle, "getOutputsInfo"));
    error = dlerror();
    if (error || getOutputsInfo == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    release_fn release = reinterpret_cast<release_fn>(dlsym(handle, "release"));
    error = dlerror();
    if (error || release == nullptr) {
        SPDLOG_LOGGER_ERROR(modelmanager_logger, "Failed to load library name: {} with error: {}", name, error);
        dlclose(handle);
        return StatusCode::NODE_LIBRARY_LOAD_FAILED_SYM;
    }

    libraries[name] = NodeLibrary{
        initialize,
        deinitialize,
        execute,
        getInputsInfo,
        getOutputsInfo,
        release,
        basePath};

    std::unique_ptr<NodeLibraryBase> nodeLibrary = std::make_unique<NodeLibraryBase>(
        basePath
    );
    libraryExecutors[name] = std::make_unique<NodeLibraryExecutor>(std::move(nodeLibrary));

    SPDLOG_LOGGER_INFO(modelmanager_logger, "Successfully loaded custom node library name: {}; base_path: {}", name, basePath);
    return StatusCode::OK;
}

Status CustomNodeLibraryManager::getLibrary(const std::string& name, NodeLibrary& library, std::shared_ptr<NodeLibraryExecutor>& libraryExecutor) const {
    auto library_it = libraries.find(name);
    if (library_it == libraries.end()) {
        return StatusCode::NODE_LIBRARY_MISSING;
    } else {
        library = library_it->second;
    }
    auto executor_it = libraryExecutors.find(name);
    if (executor_it == libraryExecutors.end()) {
        return StatusCode::NODE_LIBRARY_MISSING;
    } else {
        libraryExecutor = executor_it->second;
    }

    return StatusCode::OK;
}

void CustomNodeLibraryManager::unloadLibrariesRemovedFromConfig(const std::set<std::string>& librariesInConfig) {
    std::set<std::string> librariesCurrentlyLoaded;
    for (auto& library : libraries) {
        librariesCurrentlyLoaded.emplace(library.first);
    }
    std::set<std::string> librariesToUnload;
    std::set_difference(
        librariesCurrentlyLoaded.begin(), librariesCurrentlyLoaded.end(),
        librariesInConfig.begin(), librariesInConfig.end(),
        std::inserter(librariesToUnload, librariesToUnload.end()));
    for (auto& library : librariesToUnload) {
        libraries.erase(library);
    }
}

}  // namespace ovms
