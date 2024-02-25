#include "precompiled/pch.hpp"

#include "scene/scene.hpp"

#include "core/main.hpp"
#include "ecs/components/component_list.hpp"

namespace RDE {

Scene::Scene()
    : m_registry(std::make_unique<entt::registry>())
    , m_camera(std::make_unique<Camera>())
{}

Scene::Scene(Scene&& rhs)
    : m_registry(std::move(rhs.m_registry))
    , m_camera(std::move(rhs.m_camera))
{}

Scene& Scene::operator=(Scene&& rhs)
{
    m_registry = std::move(rhs.m_registry);
    m_camera = std::move(rhs.m_camera);
    return *this;
}

void Scene::init()
{
    static auto& assetManager = g_engine->assetManager();
    constexpr int entityCount = 100;
    int n = static_cast<decltype(n)>(std::floor(std::cbrtf(entityCount)));
    constexpr float scaling = 1.0f;
    constexpr float space = 5.0f * scaling;

    glm::vec3 trans(-n * space, -n * space, -n * space);

    [[maybe_unused]] const auto vikingModelId = assetManager.getAssetId("assets/models/viking_room.obj");
    [[maybe_unused]] const auto cubeModelId = assetManager.getAssetId("assets/models/cube.obj");
    [[maybe_unused]] const auto shuttleModelId = assetManager.getAssetId("assets/models/shuttle.obj");
    [[maybe_unused]] const auto capsuleModelId = assetManager.getAssetId("assets/models/capsule.obj");
    [[maybe_unused]] const auto beastModelId = assetManager.getAssetId("assets/models/mythical_beast.obj");
    [[maybe_unused]] const auto bodyModelId = assetManager.getAssetId("assets/models/body.obj");
    [[maybe_unused]] const auto sensorModelId = assetManager.getAssetId("assets/models/sensor.obj");
    [[maybe_unused]] const auto restModelId = assetManager.getAssetId("assets/models/rest.obj");
    [[maybe_unused]] const auto spaceshipModelId = assetManager.getAssetId("assets/models/spaceship.obj");

    [[maybe_unused]] const auto vikingTextureId = assetManager.getAssetId("assets/textures/viking_room.png");
    [[maybe_unused]] const auto rdeTextureId = assetManager.getAssetId("assets/textures/rde_texture.png");
    [[maybe_unused]] const auto portraitTextureId = assetManager.getAssetId("assets/textures/portrait.jpg");
    [[maybe_unused]] const auto capsuleTextureId = assetManager.getAssetId("assets/textures/capsule.jpg");
    [[maybe_unused]] const auto beastTextureId = assetManager.getAssetId("assets/textures/mythical_beast.png");
    [[maybe_unused]] const auto bodyTextureId = assetManager.getAssetId("assets/textures/body_DM.png");
    [[maybe_unused]] const auto sensorTextureId = assetManager.getAssetId("assets/textures/sensor_DM.png");
    [[maybe_unused]] const auto grassTextureId = assetManager.getAssetId("assets/textures/grass.png");

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

    auto body = m_registry->create();
    auto& bodyTrans = m_registry->emplace<TransformComponent>(body);
    auto& bodyModel = m_registry->emplace<MeshComponent>(body);

    bodyTrans.scale *= scaling * bodyScale;
    bodyModel.modelGuid = bodyModelId;
    bodyModel.textureGuid = bodyTextureId;

    auto sensor = m_registry->create();
    auto& sensorTrans = m_registry->emplace<TransformComponent>(sensor);
    auto& sensorModel = m_registry->emplace<MeshComponent>(sensor);

    sensorTrans.scale *= scaling * bodyScale;
    sensorModel.modelGuid = sensorModelId;
    sensorModel.textureGuid = sensorTextureId;

    auto rest = m_registry->create();
    auto& restTrans = m_registry->emplace<TransformComponent>(rest);
    auto& restModel = m_registry->emplace<MeshComponent>(rest);

    restTrans.scale *= scaling * bodyScale;
    restModel.modelGuid = restModelId;
    restModel.textureGuid = bodyTextureId;

    auto grass = m_registry->create();
    auto& grassTrans = m_registry->emplace<TransformComponent>(grass);
    auto& grassModel = m_registry->emplace<MeshComponent>(grass);

    grassTrans.scale *= scaling * cubeScale;
    grassTrans.translate += cubeOffset;
    grassModel.modelGuid = bodyModelId;
    grassModel.textureGuid = grassTextureId;

    auto grass2 = m_registry->create();
    auto& grassTrans2 = m_registry->emplace<TransformComponent>(grass2);
    auto& grassModel2 = m_registry->emplace<MeshComponent>(grass2);

    grassTrans2.scale *= scaling * cubeScale;
    grassTrans2.translate += shutterOffset;
    grassModel2.modelGuid = cubeModelId;
    grassModel2.textureGuid = vikingTextureId;

    auto spaceship = m_registry->create();
    auto& spaceshipTrans = m_registry->emplace<TransformComponent>(spaceship);
    auto& spaceshipModel = m_registry->emplace<MeshComponent>(spaceship);

    spaceshipTrans.scale *= scaling * cubeScale;
    spaceshipTrans.translate += spaceshipOffset;
    spaceshipModel.modelGuid = spaceshipModelId;
    spaceshipModel.textureGuid = capsuleTextureId;
}

void Scene::cleanup()
{
    m_registry->clear();
}

Camera& Scene::camera()
{
    return *m_camera;
}

const Camera& Scene::camera() const
{
    return *m_camera;
}

entt::registry& Scene::registry()
{
    return *m_registry;
}

entt::registry& Scene::registry() const
{
    return *m_registry;
}

} // namespace RDE
