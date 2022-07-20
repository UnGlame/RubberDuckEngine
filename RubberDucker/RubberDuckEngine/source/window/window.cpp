#include "input/input_handler.hpp"
#include "precompiled/pch.hpp"
#include "window.hpp"

namespace RDE {

void Window::init() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_GLFWwindow = glfwCreateWindow(m_width, m_height, "Rubberduck Engine",
                                    nullptr, nullptr);
    RDE_ASSERT_0(m_GLFWwindow, "Failed to create GLFW window!");

    glfwSetWindowUserPointer(m_GLFWwindow, this);
    glfwSetFramebufferSizeCallback(m_GLFWwindow, framebufferResizeCallback);
    glfwSetKeyCallback(m_GLFWwindow, InputHandler::keyInputCallback);
    glfwSetMouseButtonCallback(m_GLFWwindow, InputHandler::mouseInputCallback);
    glfwSetCursorPosCallback(m_GLFWwindow, InputHandler::mousePositionCallback);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(m_GLFWwindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    setCursorDisabled(false);
    setDisplayType(DisplayType::Windowed);
}

void Window::cleanup() {
    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}

void Window::setCursorDisabled(bool disabled) {
    if (m_cursorDisabled == disabled) {
        return;
    }

    m_cursorDisabled = disabled;

    disabled ? glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED)
             : glfwSetInputMode(m_GLFWwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::framebufferResizeCallback(GLFWwindow *window, int width,
                                       int height) {
    auto windowWrapper =
        reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
    windowWrapper->setResized(true);
    windowWrapper->setWidth(width);
    windowWrapper->setHeight(height);
}

void Window::setDisplayType(DisplayType displayType) {
    GLFWmonitor *monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(monitor);

    if (displayType == DisplayType::Windowed) {
        monitor = nullptr;
    }

    int xpos = displayType != DisplayType::Windowed
                   ? 0
                   : (mode->width - k_defaultWidth) >> 1;
    int ypos = displayType != DisplayType::Windowed
                   ? 0
                   : (mode->height - k_defaultHeight) >> 1;
    int width =
        displayType != DisplayType::Windowed ? mode->width : k_defaultWidth;
    int height =
        displayType != DisplayType::Windowed ? mode->height : k_defaultHeight;
    int refreshRate = GLFW_DONT_CARE;

    if (displayType == DisplayType::FullscreenBorderless) {
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    }

    glfwSetWindowMonitor(m_GLFWwindow, monitor, xpos, ypos, width, height,
                         refreshRate);

    m_resized = true;
    m_width = width;
    m_height = height;
    m_displayType = displayType;
}

void Window::toggleDisplayType() {
    switch (m_displayType) {
    case DisplayType::Fullscreen: {
        setDisplayType(DisplayType::FullscreenBorderless);
        break;
    }
    case DisplayType::FullscreenBorderless: {
        setDisplayType(DisplayType::Windowed);
        break;
    }
    case DisplayType::Windowed: {
        setDisplayType(DisplayType::Fullscreen);
        break;
    }
    }
}
} // namespace RDE
