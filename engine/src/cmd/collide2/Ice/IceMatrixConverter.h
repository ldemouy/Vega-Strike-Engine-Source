#include "IceMatrix3x3.h"
#include "IceMatrix4x4.h"

Matrix4x4 ConvertMatrix3x3ToMatrix4x4(Matrix3x3 m)
{
    return Matrix4x4(
        m[0][0], m[0][1], m[0][2], 0.0f,
        m[1][0], m[1][1], m[1][2], 0.0f,
        m[2][0], m[2][1], m[2][2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f);
}