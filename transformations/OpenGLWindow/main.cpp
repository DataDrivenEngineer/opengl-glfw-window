// OpenGLWindow.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cmath>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cyTriMesh.h>
#include <cyMatrix.h>
#include <cyGL.h>

#define PI 3.141592653589793238
#define DEG2RAD(degrees) degrees * (PI / 180)
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.f

static float rotationAngle = 0.f;
static float previousX = -1.f;
static float distanceFromCamera = 50.f;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
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
    if (distanceFromCamera < NEAR_PLANE) distanceFromCamera = NEAR_PLANE;
    if (distanceFromCamera > FAR_PLANE) distanceFromCamera = FAR_PLANE;
    if (distanceFromCamera >= NEAR_PLANE && distanceFromCamera <= FAR_PLANE)
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

static void init(cyGLSLProgram& prog, cy::TriMesh& mesh)
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

static void draw(cyGLSLProgram& prog, uint16_t width, uint16_t height, cy::TriMesh& mesh)
{
    static float deltaTime{};

    const cy::Matrix4f translationMatrix = cy::Matrix4f::Translation(cy::Vec3f(0, 0, -distanceFromCamera));
    const cy::Matrix3f rotXMatrix = cy::Matrix3f::RotationX(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotYMatrix = cy::Matrix3f::RotationY(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotZMatrix = cy::Matrix3f::RotationZ(DEG2RAD(rotationAngle));
    const cy::Matrix4f projMatrix = cy::Matrix4f::Perspective(DEG2RAD(40), float(width) / float(height), NEAR_PLANE, FAR_PLANE);
    prog["mvp"] = projMatrix * translationMatrix * rotZMatrix * rotXMatrix * rotYMatrix;

    glDrawArrays(GL_POINTS, 0, mesh.NV());
    deltaTime += 0.05;
}

int main()
{
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    const uint16_t width = 1980;
    const uint16_t height = 1024;
    GLFWwindow* const window = glfwCreateWindow(width, height, "OpenGLWindow", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewInit();

    printf("%s\n", glGetString(GL_VERSION));

    CY_GL_REGISTER_DEBUG_CALLBACK

    cyGLSLProgram prog;
    cy::TriMesh mesh;
    init(prog, mesh);
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        prog.Bind();
        draw(prog, width, height, mesh);
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}