#include "precompiled/pch.hpp"

#include "scene/scene.hpp"

#include "core/main.hpp"

namespace RDE {

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
    const auto sensorModelId = assetManager.getAssetID("assets/models/sensor.obj");
    const auto restModelId = assetManager.getAssetID("assets/models/rest.obj");
    const auto spaceshipModelId = assetManager.getAssetID("assets/models/spaceship.obj");

    const auto vikingTextureId = assetManager.getAssetID("assets/textures/viking_room.png");
    const auto rdeTextureId = assetManager.getAssetID("assets/textures/rde_texture.png");
    const auto portraitTextureId = assetManager.getAssetID("assets/textures/portrait.jpg");
    const auto capsuleTextureId = assetManager.getAssetID("assets/textures/capsule.jpg");
    const auto beastTextureId = assetManager.getAssetID("assets/textures/mythical_beast.png");
    const auto bodyTextureId = assetManager.getAssetID("assets/textures/body_DM.png");
    const auto sensorTextureId = assetManager.getAssetID("assets/textures/sensor_DM.png");
    const auto grassTextureId = assetManager.getAssetID("assets/textures/grass.png");

    glm::vec3 vikingOffset = {30.0f, 10.0f, -30.0f};
    glm::vec3 cubeOffset = {4.0f, 0.0f, 0.0f};
    glm::vec3 shutterOffset = {8.0f, 0.0f, 0.0f};
    glm::vec3 bodyOffset = {0.0f, 0.0f, 0.0f};

    constexpr float vikingScale = 1.0f;
    constexpr float cubeScale = 1.0f;
    constexpr float capsuleScale = 1.0f;
    constexpr float beastScale = 1.0f;
    constexpr float bodyScale = 1.0f;

    auto body = g_engine->registry().create();
    auto& bodyTrans = g_engine->registry().emplace<TransformComponent>(body);
    auto& bodyModel = g_engine->registry().emplace<MeshComponent>(body);

    bodyTrans.scale *= scaling * bodyScale;
    bodyModel.modelGuid = bodyModelId;
    bodyModel.textureGuid = bodyTextureId;

    auto sensor = g_engine->registry().create();
    auto& sensorTrans = g_engine->registry().emplace<TransformComponent>(sensor);
    auto& sensorModel = g_engine->registry().emplace<MeshComponent>(sensor);

    sensorTrans.scale *= scaling * bodyScale;
    sensorModel.modelGuid = sensorModelId;
    sensorModel.textureGuid = sensorTextureId;

    auto rest = g_engine->registry().create();
    auto& restTrans = g_engine->registry().emplace<TransformComponent>(rest);
    auto& restModel = g_engine->registry().emplace<MeshComponent>(rest);

    restTrans.scale *= scaling * bodyScale;
    restModel.modelGuid = restModelId;
    restModel.textureGuid = bodyTextureId;

    auto grass = g_engine->registry().create();
    auto& grassTrans = g_engine->registry().emplace<TransformComponent>(grass);
    auto& grassModel = g_engine->registry().emplace<MeshComponent>(grass);

    grassTrans.scale *= scaling * cubeScale;
    grassTrans.translate += cubeOffset;
    grassModel.modelGuid = bodyModelId;
    grassModel.textureGuid = grassTextureId;

    auto grass2 = g_engine->registry().create();
    auto& grassTrans2 = g_engine->registry().emplace<TransformComponent>(grass2);
    auto& grassModel2 = g_engine->registry().emplace<MeshComponent>(grass2);

    grassTrans2.scale *= scaling * cubeScale;
    grassTrans2.translate += shutterOffset;
    grassModel2.modelGuid = cubeModelId;
    grassModel2.textureGuid = vikingTextureId;

    // for (int i = 0; i < n; ++i) {
    //     for (int j = 0; j < n; ++j) {
    //         for (int k = 0; k < n; ++k) {
    //             for (int type = 0; type < 2; ++type) {
    //                 auto entity = g_engine->registry().create();
    //                 auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
    //                 auto& model = g_engine->registry().emplace<MeshComponent>(entity);

    //                switch (type) {
    //                case 0: {
    //                    transform.translate = trans + vikingOffset;
    //                    transform.scale *= scaling * vikingScale;
    //                    model.modelGuid = vikingModelId;
    //                    model.textureGuid = vikingTextureId;
    //                    break;
    //                }
    //                case 1: {
    //                    transform.translate = trans + bodyOffset;
    //                    transform.scale *= scaling * bodyScale;
    //                    model.modelGuid = bodyModelId;
    //                    model.textureGuid = bodyTextureId;

    //                    // Create another entity for sensors
    //                    entity = g_engine->registry().create();
    //                    transform = g_engine->registry().emplace<TransformComponent>(entity);
    //                    model = g_engine->registry().emplace<MeshComponent>(entity);

    //                    transform.translate = trans + bodyOffset;
    //                    transform.scale *= scaling * bodyScale;
    //                    model.modelGuid = sensorModelId;
    //                    model.textureGuid = sensorTextureId;

    //                    break;
    //                }
    //                }
    //                trans.x += 1;
    //            }
    //        }
    //        trans.y += -n * space;
    //        trans.x = -n * space;
    //    }
    //    trans.z += -n * space;
    //    trans.y = -n * space;
    //}
}
} // namespace RDE
