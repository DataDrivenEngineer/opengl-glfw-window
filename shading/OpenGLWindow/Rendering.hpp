#pragma once

#include <cyMatrix.h>
#include <cyTriMesh.h>

extern void getUniversalTransformationMatrix(cy::Matrix4f& matrixOut, const cy::TriMesh& mesh);