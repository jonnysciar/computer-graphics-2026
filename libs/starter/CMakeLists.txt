cmake_minimum_required(VERSION 3.12)

project(Starter LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${PROJECT_NAME} PRIVATE
        -DSTARTER_IMPLEMENTATION
        -DSCENE_IMPLEMENTATION
        -DTEXTMAKER_IMPLEMENTATION
        -DANIMATIONS_IMPLEMENTATION)

target_precompile_headers(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/Starter.hpp")

target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
target_link_libraries(${PROJECT_NAME} PUBLIC nlohmann_json::nlohmann_json)
