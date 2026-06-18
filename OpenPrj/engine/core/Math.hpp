#pragma once

#include <cmath>
#include "../nkMath/NkVec2.h"
#include "../nkMath/NkVec3.h"
#include "../nkMath/NkVec4.h"
#include "../nkMath/NkMat3x3.h"
#include "../nkMath/NkMat4x4.h"
#include "../nkMath/NkQuat.h"
#include "../nkMath/NkMathUtils.h"
#include "../nkMath/NkColor.h"

namespace engine {

// Type aliases for nkMath types
using Vec2f = NkVec2<float>;
using Vec3f = NkVec3<float>;
using Vec4f = NkVec4<float>;
using Mat3f = NkMat3x3<float>;
using Mat4f = NkMat4x4<float>;
using Quatf = NkQuat<float>;
using Color = NkColor;

// Common transformation matrices using nkMath
namespace Transform {
    inline Mat4f Identity() {
        return Mat4f();
    }

    inline Mat4f Translation(const Vec3f& pos) {
        return Mat4f::Translation(pos[0], pos[1], pos[2]);
    }

    inline Mat4f Scale(const Vec3f& scale) {
        return Mat4f::Scale(scale[0], scale[1], scale[2]);
    }

    inline Mat4f RotationX(float radians) {
        return Mat4f::RotationX(radians);
    }

    inline Mat4f RotationY(float radians) {
        return Mat4f::RotationY(radians);
    }

    inline Mat4f RotationZ(float radians) {
        return Mat4f::RotationZ(radians);
    }

    inline Mat4f Rotation(const Quatf& quat) {
        return quat.ToMatrix();
    }

    // Basic perspective projection matrix
    inline Mat4f Perspective(float fovRadians, float aspect, float near, float far) {
        float f = 1.0f / std::tan(fovRadians * 0.5f);
        Mat4f result;

        result(0, 0) = f / aspect;
        result(1, 1) = f;
        result(2, 2) = (far + near) / (near - far);
        result(2, 3) = (2.0f * far * near) / (near - far);
        result(3, 2) = -1.0f;
        result(3, 3) = 0.0f;

        return result;
    }

    // Basic orthographic projection matrix
    inline Mat4f Orthographic(float left, float right, float bottom, float top, float near, float far) {
        Mat4f result;

        result(0, 0) = 2.0f / (right - left);
        result(1, 1) = 2.0f / (top - bottom);
        result(2, 2) = -2.0f / (far - near);
        result(0, 3) = -(right + left) / (right - left);
        result(1, 3) = -(top + bottom) / (top - bottom);
        result(2, 3) = -(far + near) / (far - near);

        return result;
    }

    // LookAt camera matrix
    inline Mat4f LookAt(const Vec3f& eye, const Vec3f& center, const Vec3f& up) {
        Vec3f f = (center - eye).Normalize();
        Vec3f s = f.CrossProduct(up).Normalize();
        Vec3f u = s.CrossProduct(f);

        Mat4f result;
        result(0, 0) = s[0];
        result(1, 0) = s[1];
        result(2, 0) = s[2];

        result(0, 1) = u[0];
        result(1, 1) = u[1];
        result(2, 1) = u[2];

        result(0, 2) = -f[0];
        result(1, 2) = -f[1];
        result(2, 2) = -f[2];

        result(0, 3) = -s.Dot(eye);
        result(1, 3) = -u.Dot(eye);
        result(2, 3) = f.Dot(eye);

        return result;
    }
}

}
