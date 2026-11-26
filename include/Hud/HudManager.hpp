#pragma once

#include "GameStatus.hpp"
#include "Hud/HudStructs.hpp"
#include "Hud/HudText.hpp"
#include "InputManager.hpp"
#include "Listener/Listenable.hpp"
#include <glm/fwd.hpp>
#include <memory>
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

    std::unique_ptr<HudText> hudText;
    bool dirty = true;

    uint32_t countedFrame = 0;
    float elapsed = 0.0;

    GameStatus::State prevState{GameStatus::State::NONE};

    static void populateCmdBufferAccess(VkCommandBuffer cmdBuf, int i, void *params);
    static void freeCmdBufferAccess(void *params);

    void parseHudFrames(nlohmann::json &json, int w, int h);
    void parseHudElements(nlohmann::json &json);
    void buildMesh();

    void buildFPS(GameStatus &gs, float deltaT);
    void buildUI(GameStatus &gs);

  public:
    HudManager(BaseProject *bp, int w, int h);

    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void populateCommandBuffer(VkCommandBuffer cmdBuf, int currentImage);
    void updateUniformBuffer(int currentImage, GameStatus &gameStatus, float deltaT);
    void setPrompt(std::string_view text);

    void subscribeElements(Listenable<GameStatus &, MouseState> &listenable);

    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
