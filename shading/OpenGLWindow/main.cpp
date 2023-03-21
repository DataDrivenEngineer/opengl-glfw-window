#pragma once

#include <iostream>
#include <cmath>
#include <limits>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cyMatrix.h>
#include <cyTriMesh.h>
#include <cyGL.h>

#include "RenderingSettings.hpp"
#include "Rendering.hpp"

static float previousX = -1.f;
static cyGLSLProgram prog;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        bPerspective = !bPerspective;
    if (key == GLFW_KEY_F6 && action == GLFW_PRESS)
        prog.BuildFiles("vertex.vert", "fragment.frag");
}

static void LMBDragCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    const float delta = 3.f;
    if (previousX > 0.f)
        rotationAngle = xpos <= previousX ? rotationAngle - delta : rotationAngle + delta;
    previousX = xpos;
}

static void RMBDragCursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (distanceFromCamera < nearPlane) distanceFromCamera = nearPlane;
    if (distanceFromCamera > farPlane) distanceFromCamera = farPlane;
    if (distanceFromCamera >= nearPlane && distanceFromCamera <= farPlane)
    {
        const float delta = 2.f;
        if (previousX > 0.f)
            distanceFromCamera = xpos <= previousX ? distanceFromCamera - delta : distanceFromCamera + delta;
        previousX = xpos;
    }
}

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        glfwSetCursorPosCallback(window, NULL);
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwSetCursorPosCallback(window, LMBDragCursorCallback);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        glfwSetCursorPosCallback(window, NULL);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        glfwSetCursorPosCallback(window, RMBDragCursorCallback);
    }
}

static void init(cy::TriMesh& mesh)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    mesh.LoadFromFileObj("Assets/teapot.obj");
    
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * mesh.NV(), &mesh.V(0), GL_STATIC_DRAW);

    prog.BuildFiles("vertex.vert", "fragment.frag");
    prog.SetAttribBuffer("pos", vao, 3);
}

static void draw(cy::TriMesh& mesh)
{
    static float deltaTime{};

    cy::Matrix4f mvp;
    getUniversalTransformationMatrix(mvp, mesh);
    prog["mvp"] = std::move(mvp);

    glDrawArrays(GL_POINTS, 0, mesh.NV());
    deltaTime += 0.05;
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLFWwindow* const window = glfwCreateWindow(screenWidth, screenHeight, "OpenGLWindow", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewInit();

    printf("%s\n", glGetString(GL_VERSION));

    CY_GL_REGISTER_DEBUG_CALLBACK

    cy::TriMesh mesh;
    init(mesh);
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        prog.Bind();
        draw(mesh);
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}