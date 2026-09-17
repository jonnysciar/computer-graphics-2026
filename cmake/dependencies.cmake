include(FetchContent)

# Vulkan
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
    message(STATUS "Vulkan: found, version ${Vulkan_VERSION}")
endif ()

# GLFW
find_package(glfw3 3.4 QUIET)

if (glfw3_FOUND)
    message(STATUS "GLFW: found use system library, version ${glfw3_VERSION}")
else ()
    FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
    )
    # Deactivate non-essential components of GLFW to speed up the build
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glfw)
    message(STATUS "GLFW: downloaded, version 3.4.0")
endif ()

# GLM
find_package(glm QUIET)

if (glm_FOUND)
    message(STATUS "GLM: found use system library, version ${glm_VERSION}")
else ()
    FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
    )
    set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(glm)
    message(STATUS "GLM: downloaded, version 1.0.1")
endif ()

# nlohmann_json
find_package(nlohmann_json NAMES json 3.12.0 QUIET)

if (nlohmann_json_FOUND)
    message(STATUS "NLOHMANN_JSON: found use system library, version ${nlohmann_json_VERSION}")
else ()
    FetchContent_Declare(
            json
            URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
            DOWNLOAD_EXTRACT_TIMESTAMP ON
    )
    FetchContent_MakeAvailable(json)
    message(STATUS "NLOHMANN_JSON: downloaded, version 3.12.0")
endif ()

# Starter
add_subdirectory(libs/starter)