#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

namespace RDE {

    void Scene::init()
    {
        auto entity1 = g_engine->registry().create();
        auto& transform1 = g_engine->registry().emplace<TransformComponent>(entity1);
        auto& model1 = g_engine->registry().emplace<ModelComponent>(entity1);

        transform1.translate += glm::vec3(-1.0f, 0.0f, 0.0f);
        transform1.scale /= 20.0f;
        model1.modelGUID = 0; // First model loaded

        auto entity2 = g_engine->registry().create();
        auto& transform2 = g_engine->registry().emplace<TransformComponent>(entity2);
        auto& model2 = g_engine->registry().emplace<ModelComponent>(entity2);

        transform2.translate += glm::vec3(0.0f, 0.0f, 0.0f);
        transform2.scale /= 200.0f;
        model2.modelGUID = 1;

        auto entity3 = g_engine->registry().create();
        auto& transform3 = g_engine->registry().emplace<TransformComponent>(entity3);
        auto& model3 = g_engine->registry().emplace<ModelComponent>(entity3);

        transform3.translate = transform2.translate + glm::vec3(1.0f, 0.0f, 0.0f);
        transform3.scale /= 3.0f;
        model3.modelGUID = 2;
    }
}