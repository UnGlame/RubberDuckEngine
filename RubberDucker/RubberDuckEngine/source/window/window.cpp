#include "pch.hpp"
#include "window.hpp"

void RDE::Window::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_GLFWwindow = glfwCreateWindow(width, height, "Rubber Duck Engine", nullptr, nullptr);

    assert(m_GLFWwindow);
}

void RDE::Window::cleanup()
{
    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}
