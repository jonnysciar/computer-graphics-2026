#include "Scene/MainSceneHandler.hpp"

#include "HUDManager.hpp"

static void hideInstance(Instance *instance) {
    instance->Wm = glm::translate(glm::mat4(1), glm::vec3(0.0f, -1000.0f, 0.0f));
    instance->C->setWorldMatrix(instance->Wm);
}

const std::map<std::string_view, Interactable> MainSceneHandler::interMap = {
    /*{"softball",
     Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) { hideInstance(i.instance); }}},

    {"walls1", Interactable{.instance = nullptr, .callback = [](Interactable &i, Player &p) { p.pos.y += 1.0f; }}},*/
};

MainSceneHandler::MainSceneHandler(SceneLoader *sceneLoader, HUDManager *hudManager)
    : SceneHandler(sceneLoader), hudManager(hudManager) {

    std::vector<Interactable> interactables;
    interactables.reserve(interMap.size());
    for (const auto &instance : instances) {
        auto it = interMap.find(*instance->id);
        if (it == interMap.end())
            continue;

        auto inter = it->second;
        inter.instance = instance;
        interactables.push_back(std::move(inter));
    }

    interactionController = std::make_unique<InteractionController>(collisionDetector, std::move(interactables));
}

void MainSceneHandler::updateUniformBuffers(uint32_t currentImage, Player &p, glm::mat4 viewPrjMat, float deltaT,
                                            void *data) {
    UniformBufferObject ubo{};
    GlobalUniformBufferObject gubo{};

    // defines the global parameters for the uniform
    gubo.lightColor = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f);
    gubo.pointParams = glm::vec3(debugView.x, 2.0, 0.0);
    gubo.eyePos = p.pos;
    gubo.ambientUpper = glm::vec4(0.05f, 0.05f, 1.0f, 0.0f); // sky blue
    gubo.ambientLower = glm::vec4(0.0f, 0.05f, 1.0f, 0.0f);  // warm ground
    gubo.ambientDir = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);     // world up
    gubo.debugView = debugView;

    // Each instance is drawn with its own world matrix (from general_scene.json), the same one used by its collider
    for (auto *instance : instances) {
        ubo.mMat = instance->Wm;
        ubo.mvpMat = viewPrjMat * ubo.mMat;
        ubo.nMat = glm::inverse(glm::transpose(ubo.mMat));
        instance->DS[0][0]->map(currentImage, &ubo, 0);
        instance->DS[0][0]->map(currentImage, &gubo, 1);
    }
}

void MainSceneHandler::onInteractionReach(std::string_view id, Player &p) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("Press E to pick up");
}

void MainSceneHandler::offInteractionReach(std::string_view id, Player &p) {
    if (!interMap.contains(id))
        return;

    hudManager->setPrompt("");
}

void MainSceneHandler::onEvent(Player &p, Key k) {
    switch (k.glfw_key) {
    case GLFW_KEY_E:
        interactionController->interact(p);
        sceneLoader->refreshColliders();
        break;
    case GLFW_KEY_L:
        debugView.x = 0.5f;
        increase = true;
        break;
    case GLFW_KEY_M: {
        auto ins = instances | std::views::filter([](auto *inst) { return *inst->id == "softball"; });
        solid = !solid;
        std::ranges::for_each(ins, [this](auto &inst) { collisionDetector->addCollider(inst->C, solid); });
    } break;
    default:
        break;
    }
}