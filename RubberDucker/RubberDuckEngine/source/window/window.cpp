#include "pch.hpp"
#include "window.hpp"

void RDE::Window::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_GLFWwindow = glfwCreateWindow(m_width, m_height, "Rubberduck Engine", nullptr, nullptr);
    glfwSetWindowUserPointer(m_GLFWwindow, this);
    glfwSetFramebufferSizeCallback(m_GLFWwindow, framebufferResizeCallback);

    assert(m_GLFWwindow);
}

void RDE::Window::cleanup()
{
    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}

void RDE::Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto windowWrapper = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    windowWrapper->setResized(true);
    windowWrapper->setWidth(width);
    windowWrapper->setHeight(height);
}
