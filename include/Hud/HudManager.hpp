#pragma once

#include "GameStatus.hpp"
#include "HudStructs.hpp"
#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>
#include <string_view>
#include <vector>

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

    std::vector<HudSprite> sprites;
    std::map<std::string_view, HudElement> elements;

    bool dirty = false;

    static void populateCmdBufferAccess(VkCommandBuffer cmdBuf, int i, void *params);
    static void freeCmdBufferAccess(void *params);

    void generateHudSprite(std::string_view jsonFile);
    void buildMesh();

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
