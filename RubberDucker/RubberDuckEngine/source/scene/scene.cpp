#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

#include "components/transform_component.hpp"

namespace RDE {

    void Scene::init()
    {
        auto entity = g_engine->registry().create();
        auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
    }
}