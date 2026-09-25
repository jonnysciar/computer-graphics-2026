cmake_minimum_required(VERSION 3.12)

project(Starter LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC "${CMAKE_CURRENT_SOURCE_DIR}/lib.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(${PROJECT_NAME} PUBLIC -DSTARTER_IMPLEMENTATION)

target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
#target_precompile_headers(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/modules/Starter.hpp")
