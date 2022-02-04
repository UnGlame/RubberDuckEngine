#include "precompiled/pch.hpp"
#include "window.hpp"
#include "input/input.hpp"

namespace RDE {

    void Window::init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_GLFWwindow = glfwCreateWindow(m_width, m_height, "Rubberduck Engine", nullptr, nullptr);
        RDE_ASSERT_0(m_GLFWwindow, "Failed to create GLFW window!");

        glfwSetWindowUserPointer(m_GLFWwindow, this);
        glfwSetFramebufferSizeCallback(m_GLFWwindow, framebufferResizeCallback);
        glfwSetKeyCallback(m_GLFWwindow, InputManager::keyCallback);

        setFullscreen(m_fullscreen);
    }

    void Window::cleanup()
    {
        glfwDestroyWindow(m_GLFWwindow);
        glfwTerminate();
    }

    void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        auto windowWrapper = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
        windowWrapper->setResized(true);
        windowWrapper->setWidth(width);
        windowWrapper->setHeight(height);
    }

    void Window::setFullscreen(bool fullscreen)
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (!fullscreen) {
            monitor = nullptr;
        }

        int xpos = fullscreen ? 0 : (mode->width - k_defaultWidth) >> 1;
        int ypos = fullscreen ? 0 : (mode->height - k_defaultHeight) >> 1;
        int width = fullscreen ? mode->width : k_defaultWidth;
        int height = fullscreen ? mode->height : k_defaultHeight;
        int refreshRate = GLFW_DONT_CARE;

        glfwSetWindowMonitor(m_GLFWwindow, monitor, xpos, ypos, width, height, refreshRate);

        m_resized = true;
        m_width = width;
        m_height = height;
        m_fullscreen = fullscreen;
    }
}