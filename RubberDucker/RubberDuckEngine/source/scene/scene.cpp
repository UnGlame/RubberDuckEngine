#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

namespace RDE {

    void Scene::init()
    {
        auto entity = g_engine->registry().create();
        auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
        auto& model = g_engine->registry().emplace<ModelComponent>(entity);

        model.modelGUID = 0; // First model loaded
    }
}