cmake_minimum_required(VERSION 3.12)

project(computer-graphics-2026 LANGUAGES CXX)

include(cmake/settings.cmake)
include(cmake/dependencies.cmake)
include(cmake/shaders.cmake)
include(cmake/assets.cmake)

set(NAME main)

file(GLOB_RECURSE SRCS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HDRS "${CMAKE_SOURCE_DIR}/include/*.hpp")

add_executable(${NAME} ${HDRS} ${SRCS})
target_include_directories(${NAME} PRIVATE "${CMAKE_SOURCE_DIR}/include")

# Vulkan
target_link_libraries(${NAME} PRIVATE Vulkan::Vulkan)

# GLFW
target_link_libraries(${NAME} PRIVATE glfw)

# GLM
target_link_libraries(${NAME} PRIVATE glm::glm)

# nlohmann_json
target_link_libraries(${NAME} PRIVATE nlohmann_json::nlohmann_json)

# Starter
target_link_libraries(${NAME} PRIVATE Starter)

# Shaders
add_dependencies(${NAME} build_shaders)

# Assets
add_dependencies(${NAME} copy_assets)