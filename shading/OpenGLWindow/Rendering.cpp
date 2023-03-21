#include "Rendering.hpp"
#include "RenderingSettings.hpp"

#define PI 3.141592653589793238
#define DEG2RAD(degrees) degrees * (PI / 180)

static void Orthographic(cy::Matrix4f& out, const float l, const float r, const float t, const float b, const float n, const float f)
{
    out = std::move(cy::Matrix4f::Identity());
    out.cell[0] = 2.f / (r - l);
    out.cell[5] = 2.f / (t - b);
    out.cell[10] = -2.f / (f - n);
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

void getUniversalTransformationMatrix(cy::Matrix4f& matrixOut, const cy::TriMesh& mesh)
{
    const cy::Matrix4f translationMatrix = cy::Matrix4f::Translation(cy::Vec3f(0, 0, -distanceFromCamera));
    const cy::Matrix3f rotXMatrix = cy::Matrix3f::RotationX(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotYMatrix = cy::Matrix3f::RotationY(DEG2RAD(rotationAngle));
    const cy::Matrix3f rotZMatrix = cy::Matrix3f::RotationZ(DEG2RAD(rotationAngle));
    cy::Matrix3f scaleMatrix;
    cy::Matrix4f projMatrix;
    if (bPerspective)
    {
        projMatrix = std::move(cy::Matrix4f::Perspective(DEG2RAD(fov), float(screenWidth) / float(screenHeight), nearPlane, farPlane));
        scaleMatrix = std::move(cy::Matrix3f::Identity());
    }
    else
    {
        const float aspectRatio = screenWidth / float(screenHeight);
        Orthographic(projMatrix, -1.f, 1.f * aspectRatio, 1.f, -1.f, nearPlane, farPlane);
        scaleMatrix = std::move(cy::Matrix3f::Scale(cy::Vec3f(1.f / distanceFromCamera)));
    }

    cy::Vec3f minWorld, maxWorld;
    getMinMaxWorld(minWorld, maxWorld, mesh);
    const cy::Vec3f translate = -(minWorld + maxWorld) / 2.f;
    const cy::Matrix4f translateMeshOriginToWorldOrigin = cy::Matrix4f::Translation(std::move(cy::Vec3f(translate.x, translate.y, translate.z)));

    matrixOut = projMatrix * translationMatrix * rotZMatrix * rotXMatrix * rotYMatrix * scaleMatrix * translateMeshOriginToWorldOrigin;
}