#pragma once
#include "scene/scene.hpp"

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
    void saveScene(std::string_view sceneName);

    [[nodiscard]] Scene& currentScene();
    [[nodiscard]] const Scene& currentScene() const;
    [[nodiscard]] Scene& getScene(std::string_view sceneName);
    [[nodiscard]] const Scene& getScene(std::string_view sceneName) const;

private:
    std::unordered_map<std::string, Scene> m_activeScenes;
    std::string m_currentScene;
};

} // namespace RDE
