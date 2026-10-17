#pragma once

#include <modules/Starter.hpp>
#include <modules/TextMaker.hpp>

class HUDManager {
  private:
    // to provide textual feedback
    TextMaker txt;
    std::string currentPrompt;

  public:
    HUDManager(BaseProject *bp, int w, int h);

    void onResize(int w, int h);
    void pipelinesAndDescriptorSetsInit();
    void updateUniformBuffer(uint32_t currentImage, float deltaT);
    void setPrompt(std::string_view text);

    void cleanUp();
    void pipelinesAndDescriptorSetsCleanup();
};
