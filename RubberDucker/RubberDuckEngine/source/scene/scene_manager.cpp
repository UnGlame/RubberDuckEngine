#include "precompiled/pch.hpp"

#include "scene/scene_manager.hpp"

#include "ecs/components/component_list.hpp"
#include "utilities/utilities.hpp"

#include <unordered_map>
#include <vector>

namespace RDE {

void SceneManager::init()
{
    // TODO: Load scene from config
    const auto sceneName = "DefaultScene";
    auto& scene = loadScene(sceneName);
    scene.init();
}

void SceneManager::cleanup()
{
    // Save current scene?
}

Scene& SceneManager::loadScene(std::string_view sceneName)
{
    m_currentScene = sceneName;

    if (m_activeScenes.contains(std::string(sceneName))) {
        return m_activeScenes.at(std::string(sceneName));
    }

    const auto [it, _] = m_activeScenes.emplace(std::make_pair(std::string(sceneName), Scene()));
    auto& scene = it->second;
    // TODO: Load new scene from json

    return scene;
}

void SceneManager::saveCurrentScene()
{
    saveScene(m_currentScene);
}

void SceneManager::saveScene(std::string_view sceneName)
{
    auto& scene = m_activeScenes.at(std::string(sceneName));
    auto& registry = scene.registry();
    const auto& entitiesStorage = registry.storage<entt::entity>();

    nlohmann::json sceneJson{};
    nlohmann::json entitiesJson{};
    sceneJson["scene_name"] = sceneName;
    sceneJson["entity_count"] = entitiesStorage.size();

    for (const auto entity : entitiesStorage) {
        nlohmann::json entityJson{};
        nlohmann::json componentsJson{};
        entityJson["entity_id"] = entity;

        serializeEntityComponents(registry, entity, componentsJson, k_componentList);

        entityJson["components"] = componentsJson;
        entitiesJson.emplace_back(entityJson);
    }
    sceneJson["entities"] = entitiesJson;

    RDELOG_INFO("{}: {}", sceneName, sceneJson.dump(2));

    std::filesystem::path path(k_sceneDirPath);
    path /= sceneName;

    RDELOG_INFO("Saved scene {}", path.string());
}

[[nodiscard]] Scene& SceneManager::currentScene()
{
    return m_activeScenes.at(m_currentScene);
}

[[nodiscard]] const Scene& SceneManager::currentScene() const
{
    return m_activeScenes.at(m_currentScene);
}

[[nodiscard]] Scene& SceneManager::getScene(std::string_view sceneName)
{
    return m_activeScenes.at(std::string(sceneName));
}

[[nodiscard]] const Scene& SceneManager::getScene(std::string_view sceneName) const
{
    return m_activeScenes.at(std::string(sceneName));
}

} // namespace RDE
