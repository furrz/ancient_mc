#pragma once
#include <GLFW/glfw3.h>

class Input
{
    static GLFWwindow *window_;

public:

    static void setWindow(GLFWwindow *window)
    {
        window_ = window;
    }

    [[nodiscard]] static bool getKey(const int key)
    {
        return glfwGetKey(window_, key) == GLFW_PRESS;
    }
};
