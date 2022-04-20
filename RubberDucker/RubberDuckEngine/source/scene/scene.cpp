#include "precompiled/pch.hpp"
#include "scene/scene.hpp"

namespace RDE {

    void Scene::init()
    {
        static auto& assetManager = g_engine->assetManager();
        constexpr int entityCount = 1;
        int n = static_cast<decltype(n)>(std::floor(std::cbrtf(entityCount)));
        constexpr float scaling = 20.0f;
        glm::vec3 trans(-n / scaling, -n / scaling, -n / scaling);

        auto vikingId = assetManager.getAssetID("assets/models/viking_room.obj");
        auto cubeId = assetManager.getAssetID("assets/models/cube.obj");
        auto shuttleId = assetManager.getAssetID("assets/models/shuttle.obj");

        
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
        //        model.modelGUID = vikingId;
        //
        //        trans.x += 1;
        //    }
        //    trans.z += 1;
        //    trans.x = -n / scaling;
        //}
        
        trans = glm::vec3(-n / scaling, -n / scaling, -n / scaling);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = 0; k < n; ++k) {
                    auto entity = g_engine->registry().create();
                    auto& transform = g_engine->registry().emplace<TransformComponent>(entity);
                    auto& model = g_engine->registry().emplace<ModelComponent>(entity);

                    transform.translate = trans;
                    transform.scale /= scaling * 0.1f;
                    model.modelGUID = cubeId;

                    trans.x += 1;
                }
                trans.y += 1;
                trans.x = -n / scaling;
            }
            trans.z += 1;
            trans.y = -n / scaling;
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