#pragma once

#include <vector>
#include <cstdint>

struct Instance;

class SceneHandler {
protected:
    std::vector<Instance *> instances;
public:
    explicit SceneHandler(std::vector<Instance *> instances) : instances(std::move(instances)) {}
    virtual ~SceneHandler() = default;
    virtual void updateUniformBuffers(uint32_t currentImage, void* data) = 0;
};
