#include "scene/scene.hpp"
#include "precompiled/pch.hpp"

namespace RDE
{

void Scene::init()
{
    static auto& assetManager = g_engine->assetManager();
    constexpr int entityCount = 100;
    int n = static_cast<decltype(n)>(std::floor(std::cbrtf(entityCount)));
    constexpr float scaling = 8.0f;
    glm::vec3 trans(-n / scaling, -n / scaling, -n / scaling);

    const auto vikingModelId = assetManager.getAssetID("assets/models/viking_room.obj");
    const auto cubeModelId = assetManager.getAssetID("assets/models/cube.obj");
    const auto shuttleModelId = assetManager.getAssetID("assets/models/shuttle.obj");
    const auto capsuleModelId = assetManager.getAssetID("assets/models/capsule.obj");
    const auto beastModelId = assetManager.getAssetID("assets/models/mythical_beast.obj");

    const auto vikingTextureId = assetManager.getAssetID("assets/textures/viking_room.png");
    const auto rdeTextureId = assetManager.getAssetID("assets/textures/rde_texture.png");
    const auto portraitTextureId = assetManager.getAssetID("assets/textures/portrait.jpg");
    const auto capsuleTextureId = assetManager.getAssetID("assets/textures/capsule.jpg");
    const auto beastTextureId = assetManager.getAssetID("assets/textures/mythical_beast.png");

    glm::vec3 vikingOffset = {30.0f, 10.0f, -30.0f};
    glm::vec3 cubeOffset = {50.0f, 0.0f, 0.0f};
    glm::vec3 shutterOffset = {0.0f, 0.0f, -30.0f};

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
                        transform.scale /= scaling * 0.1f;
                        model.modelGUID = vikingModelId;
                        model.textureGUID = vikingTextureId;
                        break;
                    }
                    case 1: {
                        transform.translate = trans + cubeOffset;
                        transform.scale /= scaling * 0.2f;
                        model.modelGUID = capsuleModelId;
                        model.textureGUID = capsuleTextureId;
                        break;
                    }
                    case 2: {
                        transform.translate = trans + shutterOffset;
                        transform.scale /= scaling * 15.0f;
                        model.modelGUID = beastModelId;
                        model.textureGUID = beastTextureId;
                        break;
                    }
                    }
                    trans.x += 1;
                }
            }
            trans.y += 1;
            trans.x = -n / scaling;
        }
        trans.z += 1;
        trans.y = -n / scaling;
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
