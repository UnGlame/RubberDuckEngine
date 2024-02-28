#pragma once
#include "scene/scene.hpp"
#include "serialization/serialization.hpp"

#include <entt/entt.hpp>
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
    nlohmann::ordered_json serializeEachEntity(const entt::registry& registry, std::string_view sceneName) const;

    template<typename TComponent>
    void serializeEntityComponent(const entt::registry& registry,
                                  entt::entity entity,
                                  nlohmann::ordered_json& componentsJson) const
    {
        // Try to get the component from the registry
        const TComponent* component = registry.try_get<TComponent>(entity);
        const rttr::type reflectedComponent = rttr::type::get<TComponent>();

        if (component) {
            nlohmann::ordered_json componentJson{};
            componentJson["name"] = reflectedComponent.get_name();
            componentJson["data"] = Serialization::serialize(component);
            componentsJson.emplace_back(componentJson);
        }
    }

    // Recursive function to iterate over component types
    template<typename... TComponents>
    void serializeEachEntityComponent(const entt::registry& registry,
                                      entt::entity entity,
                                      nlohmann::ordered_json& componentsJson,
                                      entt::type_list<TComponents...>) const
    {
        // Call implementation on each component using fold expression
        ((serializeEntityComponent<TComponents>(registry, entity, componentsJson)), ...);
    }

    std::unordered_map<std::string, Scene> m_activeScenes;
    std::string m_currentScene;

    static constexpr const char* k_sceneDirPath = "assets/scenes/";
};

} // namespace RDE
