// OpenGLWindow.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cmath>
#include <limits>

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
static bool bPerspective = true;
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

static void Orthographic(cy::Matrix4f& out, const float l, const float r, const float t, const float b, const float n, const float f)
{
    out = std::move(cy::Matrix4f::Identity());
    out.cell[0] = 2.f / (r - l);
    out.cell[5] = 2.f / (t - b);
    out.cell[10] = -2.f / (f -  n);
    out.cell[12] = -(r + l) / (r - l);
    out.cell[13] = -(t + b) / (t - b);
    out.cell[14] = -(f + n) / (f - n);
}

static void getMinMaxWorld(cy::Vec3f& minWorld, cy::Vec3f& maxWorld, const cy::TriMesh& mesh)
{
    constexpr float infinity = std::numeric_limits<float>::infinity();
    minWorld.Set(infinity);
    maxWorld.Set(-infinity);
    for (unsigned short i = 0; i < mesh.NV(); ++i)
    {
        const cy::Vec3f v = mesh.V(i);
        minWorld.x = v.x < minWorld.x ? v.x : minWorld.x;
        minWorld.y = v.y < minWorld.y ? v.y : minWorld.y;
        minWorld.z = v.z < minWorld.z ? v.z : minWorld.z;
        
        maxWorld.x = v.x > maxWorld.x ? v.x : maxWorld.x;
        maxWorld.y = v.y > maxWorld.y ? v.y : maxWorld.y;
        maxWorld.z = v.z > maxWorld.z ? v.z : maxWorld.z;
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

static void draw(uint16_t width, uint16_t height, cy::TriMesh& mesh)
{
    static float deltaTime{};

    const cy::Matrix4f translationMatrix = cy::Matrix4f::Translation(cy::Vec3f(0, 0, -distanceFromCamera));
    const cy::Matrix3f rotXMatrix = cy::Matrix3f::RotationX(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotYMatrix = cy::Matrix3f::RotationY(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotZMatrix = cy::Matrix3f::RotationZ(DEG2RAD(rotationAngle));
    cy::Matrix3f scaleMatrix;
    cy::Matrix4f projMatrix;
    if (bPerspective)
    {
        projMatrix = std::move(cy::Matrix4f::Perspective(DEG2RAD(40), float(width) / float(height), NEAR_PLANE, FAR_PLANE));
        scaleMatrix = std::move(cy::Matrix3f::Identity());
    }
    else
    {
        const float aspectRatio = width / float(height);
        Orthographic(projMatrix, -1.f, 1.f * aspectRatio, 1.f, -1.f, NEAR_PLANE, FAR_PLANE);
        scaleMatrix = std::move(cy::Matrix3f::Scale(cy::Vec3f(1.f / distanceFromCamera)));
    }

    cy::Vec3f minWorld, maxWorld;
    getMinMaxWorld(minWorld, maxWorld, mesh);
    const cy::Vec3f translate = -(minWorld + maxWorld) / 2.f;
    const cy::Matrix4f translateMeshoriginToWorldOrigin = cy::Matrix4f::Translation(std::move(cy::Vec3f(translate.x, translate.y, translate.z)));

    prog["mvp"] = projMatrix * translationMatrix * rotZMatrix * rotXMatrix * rotYMatrix * scaleMatrix * translateMeshoriginToWorldOrigin;

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

    cy::TriMesh mesh;
    init(mesh);
    
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseCallback);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        prog.Bind();
        draw(width, height, mesh);
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}