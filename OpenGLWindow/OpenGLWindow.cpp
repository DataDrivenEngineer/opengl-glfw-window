// OpenGLWindow.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cmath>
#include <GLFW/glfw3.h>

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void draw()
{
    static float deltaTime{0.f};
    glClearColor(cos(sin(deltaTime)), sin(deltaTime), cos(deltaTime), 0);
    deltaTime += 0.005;
}

int main()
{
    if (!glfwInit()) return -1;

    GLFWwindow* const window = glfwCreateWindow(1980, 1024, "OpenGLWindow", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        draw();
        glfwSetKeyCallback(window, keyCallback);
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}