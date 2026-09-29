cmake_minimum_required(VERSION 3.12)

project(Starter LANGUAGES CXX)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${PROJECT_NAME} INTERFACE -DSTARTER_IMPLEMENTATION)

target_precompile_headers(${PROJECT_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/modules/Starter.hpp")

target_link_libraries(${PROJECT_NAME} INTERFACE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} INTERFACE glfw)
target_link_libraries(${PROJECT_NAME} INTERFACE glm::glm)
target_link_libraries(${PROJECT_NAME} INTERFACE nlohmann_json::nlohmann_json)
