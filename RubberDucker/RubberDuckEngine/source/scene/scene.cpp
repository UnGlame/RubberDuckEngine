#include "scene/scene.hpp"
#include "precompiled/pch.hpp"

namespace RDE
{

void Scene::init()
{
    static auto& assetManager = g_engine->assetManager();
    constexpr int entityCount = 100;
    int n = static_cast<decltype(n)>(std::floor(std::cbrtf(entityCount)));
    constexpr float scaling = 1.0f;
    constexpr float space = 5.0f * scaling;

    glm::vec3 trans(-n * space, -n * space, -n * space);

    const auto vikingModelId = assetManager.getAssetID("assets/models/viking_room.obj");
    const auto cubeModelId = assetManager.getAssetID("assets/models/cube.obj");
    const auto shuttleModelId = assetManager.getAssetID("assets/models/shuttle.obj");
    const auto capsuleModelId = assetManager.getAssetID("assets/models/capsule.obj");
    const auto beastModelId = assetManager.getAssetID("assets/models/mythical_beast.obj");
    const auto bodyModelId = assetManager.getAssetID("assets/models/body.obj");

    const auto vikingTextureId = assetManager.getAssetID("assets/textures/viking_room.png");
    const auto rdeTextureId = assetManager.getAssetID("assets/textures/rde_texture.png");
    const auto portraitTextureId = assetManager.getAssetID("assets/textures/portrait.jpg");
    const auto capsuleTextureId = assetManager.getAssetID("assets/textures/capsule.jpg");
    const auto beastTextureId = assetManager.getAssetID("assets/textures/mythical_beast.png");
    const auto bodyTextureId = assetManager.getAssetID("assets/textures/body_DM.png");

    glm::vec3 vikingOffset = {30.0f, 10.0f, -30.0f};
    glm::vec3 cubeOffset = {50.0f, 0.0f, 0.0f};
    glm::vec3 shutterOffset = {0.0f, 0.0f, -30.0f};
    glm::vec3 bodyOffset = {0.0f, 0.0f, 0.0f};

    constexpr float vikingScale = 1.0f;
    constexpr float cubeScale = 1.0f;
    constexpr float capsuleScale = 1.0f;
    constexpr float beastScale = 1.0f;
    constexpr float bodyScale = 1.0f;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k) {
                for (int type = 0; type < 3; ++type) {
                    auto entity = g_engine->registry().create();
                    auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
                    auto& model = g_engine->registry().emplace<ModelComponent>(entity);

                    switch (type) {
                    case 0: {
                        transform.translate = trans + vikingOffset;
                        transform.scale *= scaling * vikingScale;
                        model.modelGUID = vikingModelId;
                        model.textureGUID = vikingTextureId;
                        break;
                    }
                    case 1: {
                        transform.translate = trans + cubeOffset;
                        transform.scale *= scaling * vikingScale;
                        model.modelGUID = capsuleModelId;
                        model.textureGUID = capsuleTextureId;
                        break;
                    }
                    case 2: {
                        transform.translate = trans + bodyOffset;
                        transform.scale *= scaling * bodyScale;
                        model.modelGUID = bodyModelId;
                        model.textureGUID = bodyTextureId;
                        break;
                    }
                    }
                    trans.x += 1;
                }
            }
            trans.y += -n * space;
            trans.x = -n * space;
        }
        trans.z += -n * space;
        trans.y = -n * space;
    }

    // auto entity2 = g_engine->registry().create();
    // auto& transform2 =
    // g_engine->registry().emplace<TransformComponent>(entity2); auto& model2 =
    // g_engine->registry().emplace<ModelComponent>(entity2);
    //
    // transform2.translate += glm::vec3(0.0f, 0.0f, 0.0f);
    // transform2.scale /= 200.0f;
    // model2.modelGUID = 1;
    //
    // auto entity3 = g_engine->registry().create();
    // auto& transform3 =
    // g_engine->registry().emplace<TransformComponent>(entity3); auto& model3 =
    // g_engine->registry().emplace<ModelComponent>(entity3);
    //
    // transform3.translate = transform2.translate + glm::vec3(1.0f, 0.0f,
    // 0.0f); transform3.scale /= 3.0f; model3.modelGUID = 2;
}
} // namespace RDE
