#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

namespace RDE {

    void Scene::init()
    {
        auto entity1 = g_engine->registry().create();
        auto& transform1 = g_engine->registry().emplace<TransformComponent>(entity1);
        auto& model1 = g_engine->registry().emplace<ModelComponent>(entity1);

        transform1.translate = transform1.translate + glm::vec3(-1.0f, 0.0f, 0.0f);
        model1.modelGUID = 0; // First model loaded

        auto entity2 = g_engine->registry().create();
        auto& transform2 = g_engine->registry().emplace<TransformComponent>(entity2);
        auto& model2 = g_engine->registry().emplace<ModelComponent>(entity2);

        transform2.translate = transform2.translate + glm::vec3(1.0f, 0.0f, 0.0f);
        model2.modelGUID = 0;
    }
}