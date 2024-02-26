#include "precompiled/pch.hpp"

#include "utilities/utilities.hpp"

#include <unordered_map>
#include <vector>

namespace RDE {

std::vector<entt::type_info> RDE::Utilities::getComponentTypeInfos(entt::registry& registry, entt::entity entity)
{
    std::vector<entt::type_info> typeInfos{};
    auto componentStorage = registry.storage();

    for (auto&& [componentId, component] : componentStorage) {
        if (!component.contains(entity)) {
            continue;
        }
        RDELOG_INFO("Entity {} has component {}, hash {}, index {}",
                    entity,
                    component.type().name(),
                    component.type().hash(),
                    component.type().index());
        typeInfos.emplace_back(component.type());
    }

    return typeInfos;
}

std::unordered_map<entt::entity, std::vector<entt::type_info>> RDE::Utilities::getAllEntityComponentTypeInfos(
    entt::registry& registry)
{
    std::unordered_map<entt::entity, std::vector<entt::type_info>> typeInfos{};
    auto componentStorage = registry.storage();
    auto& entityStorage = registry.storage<entt::entity>();

    // O(n * k) where n is number of entities and k is number of components
    for (const auto entity : entityStorage) {
        typeInfos[entity] = getComponentTypeInfos(registry, entity);
    }

    return typeInfos;
}

} // namespace RDE
