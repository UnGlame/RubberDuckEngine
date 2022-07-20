#pragma once
// This must be included before GLFW to prevent redefinition of APIENTRY
#include <Windows.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace RDE {

class Window {
  public:
    enum class DisplayType { Windowed, FullscreenBorderless, Fullscreen };

    void init();
    void cleanup();

    inline GLFWwindow *apiWindow() const { return m_GLFWwindow; }
    inline bool isResized() const { return m_resized; }
    inline void setResized(bool resized) { m_resized = resized; }

    void setCursorDisabled(bool disabled);
    inline bool isCursorDisabled() const { return m_cursorDisabled; }
    inline void toggleCursorDisabled() { setCursorDisabled(!m_cursorDisabled); }

    template <typename T> inline T width() const {
        return static_cast<T>(m_width);
    }

    template <typename T> inline T height() const {
        return static_cast<T>(m_height);
    }

    template <typename T> inline void setWidth(T width) {
        m_width = static_cast<uint32_t>(width);
        m_resized = true;
    }

    template <typename T> inline void setHeight(T height) {
        m_height = static_cast<uint32_t>(height);
        m_resized = true;
    }

    // Callback used to update renderer's framebuffer after resize
    static void framebufferResizeCallback(GLFWwindow *window, int width,
                                          int height);

    void setDisplayType(DisplayType fullscreen);
    void toggleDisplayType();

  private:
    static constexpr uint32_t k_defaultWidth = 1600;
    static constexpr uint32_t k_defaultHeight = 900;

    GLFWwindow *m_GLFWwindow;

    bool m_resized = false;
    bool m_cursorDisabled = false;

    DisplayType m_displayType = DisplayType::Windowed;

    uint32_t m_width = k_defaultWidth;
    uint32_t m_height = k_defaultHeight;
};
} // namespace RDE
