#include "Pch.h"
#include "Window.hpp"

void RD::Window::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_GLFWwindow = glfwCreateWindow(width, height, "Rubber Duck Engine", nullptr, nullptr);

    assert(m_GLFWwindow);
}

void RD::Window::cleanup()
{
    glfwDestroyWindow(m_GLFWwindow);
    glfwTerminate();
}
