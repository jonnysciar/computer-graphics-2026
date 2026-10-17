#include "HUDManager.hpp"

HUDManager::HUDManager(BaseProject *bp, int w, int h) {
    txt.init(bp, w, h);

    txt.print(1.0f, 1.0f, "FPS:", 1, "CO", false, false, true, TAL_RIGHT, TRH_RIGHT, TRV_BOTTOM,
              {1.0f, 0.0f, 0.0f, 1.0f}, {0.8f, 0.8f, 0.0f, 1.0f});
}

void HUDManager::onResize(int w, int h) {
    txt.resizeScreen(w, h);
}

void HUDManager::pipelinesAndDescriptorSetsInit() {
    txt.pipelinesAndDescriptorSetsInit();
}

void HUDManager::updateUniformBuffer(uint32_t currentImage, float deltaT) {
    // updates the FPS counter
    static float elapsedT = 0.0f;
    static int countedFrames = 0;
    countedFrames++;
    elapsedT += deltaT;
    if (elapsedT > 1.0f) {
        float Fps = (float) countedFrames / elapsedT;
        std::ostringstream oss;
        oss << "FPS: " << Fps << "\n";
        txt.print(1.0f, 1.0f, oss.str(), 1, "CO", false, false, true, TAL_RIGHT, TRH_RIGHT, TRV_BOTTOM,
                  {1.0f, 0.0f, 0.0f, 1.0f}, {0.8f, 0.8f, 0.0f, 1.0f});
        elapsedT = 0.0f;
        countedFrames = 0;
    }

    txt.updateCommandBuffer();
}

void HUDManager::setPrompt(std::string_view text) {
    if (text == currentPrompt) {
        return;
    }
    currentPrompt = text;

    if (text.empty()) {
        txt.removeText(2);
    } else {
        txt.print(0.0f, 0.9f, text.data(), 2, "CO", false, false, true, TAL_CENTER, TRH_CENTER, TRV_BOTTOM,
                  {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f});
    }
}

void HUDManager::cleanUp() {
    txt.localCleanup();
}

void HUDManager::pipelinesAndDescriptorSetsCleanup() {
    txt.pipelinesAndDescriptorSetsCleanup();
}
