#pragma once

#include "GameStatus.hpp"
#include "Hud/HudStructs.hpp"
#include "Hud/HudText.hpp"
#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>
#include <string>
#include <string_view>

class HudManager {
  private:
    struct CmdBufStruct {
        HudManager *hudManager = nullptr;
        Model *mesh = nullptr;
    };

    BaseProject *bp;
    RenderPass rp;
    Pipeline p;
    VertexDescriptor vd;
    DescriptorSet ds;
    DescriptorSetLayout dsl;

    Model *m;
    Texture tex;

    std::map<std::string, HudFrame> frames;
    std::map<std::string, HudElement> elements;

    HudText hudText{"assets/textures/hud/font/font.json"};
    bool dirty = false;

    uint32_t countedFrame = 0;
    float elapsed = 0.0;

    static void populateCmdBufferAccess(VkCommandBuffer cmdBuf, int i, void *params);
    static void freeCmdBufferAccess(void *params);

    void parseHudFrames(nlohmann::json &json);
    void parseHudElements(nlohmann::json &json);
    void buildMesh();

    void computeFPS(float deltaT);

  public:
    HudManager(BaseProject *bp, int w, int h);

    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void populateCommandBuffer(VkCommandBuffer cmdBuf, int currentImage);
    void updateUniformBuffer(int currentImage, GameStatus gameStatus, float deltaT);
    void setPrompt(std::string_view text);
    void setStatus(std::string_view text);

    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
