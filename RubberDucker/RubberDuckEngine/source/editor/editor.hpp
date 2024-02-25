#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace RDE {

class Editor
{
public:
    void init();
    void update();

    [[nodiscard]] _forceinline bool renderingEnabled() const { return m_renderingEnabled; }

    __forceinline void enableRendering(bool enable) { m_renderingEnabled = enable; }

    inline void toggle() { m_renderingEnabled = !m_renderingEnabled; }

private:
    void newFrame() const;
    void showDockSpace() const;
    void showHierarchy();
    void showInspector();
    void showDebugInfo();

    glm::vec3 m_eulerAngles{};
    float m_dtTimer = 0.0f;
    float m_dtToDisplay = 1.0f;
    entt::entity m_selectedEntity = entt::null;
    bool m_renderingEnabled = true;
};
} // namespace RDE
