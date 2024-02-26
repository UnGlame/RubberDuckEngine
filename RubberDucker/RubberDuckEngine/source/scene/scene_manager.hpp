#pragma once
#include "scene/scene.hpp"

#include <nlohmann/json.hpp>
#include <rttr/type.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace RDE {

class SceneManager
{
public:
    void init();
    void cleanup();

    Scene& loadScene(std::string_view sceneName);

    void saveCurrentScene();
    void saveScene(std::string_view sceneName);

    [[nodiscard]] Scene& currentScene();
    [[nodiscard]] const Scene& currentScene() const;
    [[nodiscard]] Scene& getScene(std::string_view sceneName);
    [[nodiscard]] const Scene& getScene(std::string_view sceneName) const;

private:
    // Base case for recursion
    template<typename TComponent>
    void serializeEntityComponents(const entt::registry& registry,
                                   entt::entity entity,
                                   nlohmann::json& componentsJson,
                                   entt::type_list<TComponent>)
    {
        // Try to get the component from the registry
        const auto* component = registry.try_get<TComponent>(entity);
        const auto componentTypeRttr = rttr::type::get<TComponent>();
        RDELOG_INFO("Checking {}", componentTypeRttr.get_name());

        if (component) {
            nlohmann::json componentJson{};
            componentJson["component_name"] = componentTypeRttr.get_name();
            componentsJson.emplace_back(componentJson);
        }
    }

    // Recursive function to iterate over component types
    template<typename TComponent, typename... TRest>
    void serializeEntityComponents(const entt::registry& registry,
                                   entt::entity entity,
                                   nlohmann::json& componentsJson,
                                   entt::type_list<TComponent, TRest...>)
    {
        // Try to get the component from the registry
        const auto* component = registry.try_get<TComponent>(entity);
        const auto componentTypeRttr = rttr::type::get<TComponent>();
        RDELOG_INFO("Checking {}", componentTypeRttr.get_name());

        if (component) {
            nlohmann::json componentJson{};
            componentJson["component_name"] = componentTypeRttr.get_name();
            componentsJson.emplace_back(componentJson);
        }

        // Recur with the next type
        serializeEntityComponents<TRest...>(registry, entity, componentsJson, entt::type_list<TRest...>());
    }

    std::unordered_map<std::string, Scene> m_activeScenes;
    std::string m_currentScene;

    static constexpr const char* k_sceneDirPath = "assets/scenes/";
};

} // namespace RDE

/* Example scene json structure:
{
    "scene_name": "scene",
    "entity_count": 123,
    "entities": [
        {
            "entity_id": 1,
            "components": [
                {
                    "component_name": "EntityComponent",
                    "entity_name": "entity"
                },
                {
                    "component_name": "TransformComponent",
                    "scale": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "rotate": {
                        "w": 1,
                        "x": 0,
                        "y": 0,
                        "z": 0
                    },
                    "translate": {
                        "x": 0,
                        "y": 0,
                        "z": 0
                    }
                },
                {
                    "component_name": "MeshComponent",
                    "mesh_id": 1,
                    "texture_id": 2
                }
            ]
        },
        {
            "entity_id": 2,
            "components": [
                {
                    "component_name": "EntityComponent",
                    "entity_name": "entity_2"
                },
                {
                    "component_name": "TransformComponent",
                    "scale": {
                        "x": 1,
                        "y": 0,
                        "z": 0
                    },
                    "rotate": {
                        "w": 1,
                        "x": 2,
                        "y": 0,
                        "z": 0
                    },
                    "translate": {
                        "x": 0,
                        "y": 3,
                        "z": 0
                    }
                },
                {
                    "component_name": "MeshComponent",
                    "mesh_id": 2,
                    "texture_id": 3
                }
            ]
        }
    ]
}
*/
