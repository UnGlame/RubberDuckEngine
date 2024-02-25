#include "precompiled/pch.hpp"

#include "scene/scene.hpp"

#include "core/main.hpp"
#include "ecs/components/component_list.hpp"

namespace RDE {

void Scene::init()
{
    static auto& assetManager = g_engine->assetManager();
    constexpr int entityCount = 100;
    int n = static_cast<decltype(n)>(std::floor(std::cbrtf(entityCount)));
    constexpr float scaling = 1.0f;
    constexpr float space = 5.0f * scaling;

    glm::vec3 trans(-n * space, -n * space, -n * space);

    [[maybe_unused]] const auto vikingModelId = assetManager.getAssetID("assets/models/viking_room.obj");
    [[maybe_unused]] const auto cubeModelId = assetManager.getAssetID("assets/models/cube.obj");
    [[maybe_unused]] const auto shuttleModelId = assetManager.getAssetID("assets/models/shuttle.obj");
    [[maybe_unused]] const auto capsuleModelId = assetManager.getAssetID("assets/models/capsule.obj");
    [[maybe_unused]] const auto beastModelId = assetManager.getAssetID("assets/models/mythical_beast.obj");
    [[maybe_unused]] const auto bodyModelId = assetManager.getAssetID("assets/models/body.obj");
    [[maybe_unused]] const auto sensorModelId = assetManager.getAssetID("assets/models/sensor.obj");
    [[maybe_unused]] const auto restModelId = assetManager.getAssetID("assets/models/rest.obj");
    [[maybe_unused]] const auto spaceshipModelId = assetManager.getAssetID("assets/models/spaceship.obj");

    [[maybe_unused]] const auto vikingTextureId = assetManager.getAssetID("assets/textures/viking_room.png");
    [[maybe_unused]] const auto rdeTextureId = assetManager.getAssetID("assets/textures/rde_texture.png");
    [[maybe_unused]] const auto portraitTextureId = assetManager.getAssetID("assets/textures/portrait.jpg");
    [[maybe_unused]] const auto capsuleTextureId = assetManager.getAssetID("assets/textures/capsule.jpg");
    [[maybe_unused]] const auto beastTextureId = assetManager.getAssetID("assets/textures/mythical_beast.png");
    [[maybe_unused]] const auto bodyTextureId = assetManager.getAssetID("assets/textures/body_DM.png");
    [[maybe_unused]] const auto sensorTextureId = assetManager.getAssetID("assets/textures/sensor_DM.png");
    [[maybe_unused]] const auto grassTextureId = assetManager.getAssetID("assets/textures/grass.png");

    [[maybe_unused]] glm::vec3 vikingOffset = {30.0f, 10.0f, -30.0f};
    [[maybe_unused]] glm::vec3 cubeOffset = {4.0f, 0.0f, 0.0f};
    [[maybe_unused]] glm::vec3 shutterOffset = {8.0f, 0.0f, 0.0f};
    [[maybe_unused]] glm::vec3 bodyOffset = {0.0f, 0.0f, 0.0f};
    [[maybe_unused]] glm::vec3 spaceshipOffset = {-5.0f, 0.0f, 0.0f};

    [[maybe_unused]] constexpr float vikingScale = 1.0f;
    [[maybe_unused]] constexpr float cubeScale = 1.0f;
    [[maybe_unused]] constexpr float capsuleScale = 1.0f;
    [[maybe_unused]] constexpr float beastScale = 1.0f;
    [[maybe_unused]] constexpr float bodyScale = 1.0f;

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

    auto spaceship = g_engine->registry().create();
    auto& spaceshipTrans = g_engine->registry().emplace<TransformComponent>(spaceship);
    auto& spaceshipModel = g_engine->registry().emplace<MeshComponent>(spaceship);

    spaceshipTrans.scale *= scaling * cubeScale;
    spaceshipTrans.translate += spaceshipOffset;
    spaceshipModel.modelGuid = spaceshipModelId;
    spaceshipModel.textureGuid = capsuleTextureId;
}
} // namespace RDE
