#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

namespace RDE {

    void Scene::init()
    {
        static auto& assetManager = g_engine->assetManager();
        constexpr int n = 500;
        constexpr float scaling = 20.0f;
        glm::vec3 trans(-n / scaling, 0, -n / scaling);

        //auto id = assetManager.getAssetID("assets/models/viking_room.obj");
        //
        //for (int i = 0; i < n; ++i) {
        //    for (int j = 0; j < n; ++j) {
        //        auto entity = g_engine->registry().create();
        //        auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
        //        auto& model = g_engine->registry().emplace<ModelComponent>(entity);
        //
        //        transform.translate = trans;
        //        transform.scale /= scaling / 10.0f;
        //        transform.rotate = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        //        transform.rotate *= glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        //        model.modelGUID = id;
        //
        //        trans.x += 1;
        //    }
        //    trans.z += 1;
        //    trans.x = -n / scaling;
        //}
        
        auto id = assetManager.getAssetID("assets/models/cube.obj");
        trans = glm::vec3(-n / scaling, -5.0f, -n / scaling);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                auto entity = g_engine->registry().create();
                auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
                auto& model = g_engine->registry().emplace<ModelComponent>(entity);

                transform.translate = trans;
                transform.scale /= scaling / 10.0f;
                model.modelGUID = id;

                trans.x += 1;
            }
            trans.z += 1;
            trans.x = -n / scaling;
        }

        //auto entity2 = g_engine->registry().create();
        //auto& transform2 = g_engine->registry().emplace<TransformComponent>(entity2);
        //auto& model2 = g_engine->registry().emplace<ModelComponent>(entity2);
        //
        //transform2.translate += glm::vec3(0.0f, 0.0f, 0.0f);
        //transform2.scale /= 200.0f;
        //model2.modelGUID = 1;
        //
        //auto entity3 = g_engine->registry().create();
        //auto& transform3 = g_engine->registry().emplace<TransformComponent>(entity3);
        //auto& model3 = g_engine->registry().emplace<ModelComponent>(entity3);
        //
        //transform3.translate = transform2.translate + glm::vec3(1.0f, 0.0f, 0.0f);
        //transform3.scale /= 3.0f;
        //model3.modelGUID = 2;
    }
}