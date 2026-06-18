#pragma once

#include "Math.hpp"

namespace engine {

class Transform {
public:
    Transform()
        : position(0, 0, 0), scale(1, 1, 1), rotation() {}

    Transform(const Vec3f& pos)
        : position(pos), scale(1, 1, 1), rotation() {}

    Transform(const Vec3f& pos, const Quatf& rot, const Vec3f& scl)
        : position(pos), scale(scl), rotation(rot) {}

    Vec3f position;
    Quatf rotation;
    Vec3f scale;

    Mat4f GetMatrix() const {
        Mat4f translationMat = engine::Transform::Translation(position);
        Mat4f rotationMat = engine::Transform::Rotation(rotation);
        Mat4f scaleMat = engine::Transform::Scale(scale);
        return translationMat * rotationMat * scaleMat;
    }

    Vec3f Forward() const {
        return rotation.Forward().Normalize();
    }

    Vec3f Right() const {
        return rotation.Right().Normalize();
    }

    Vec3f Up() const {
        return rotation.Up().Normalize();
    }

    void Rotate(float x, float y, float z) {
        Quatf rotX = Quatf::RotateX(x);
        Quatf rotY = Quatf::RotateY(y);
        Quatf rotZ = Quatf::RotateZ(z);
        rotation = rotX * (rotY * rotZ);
    }

    void RotateAround(const Vec3f& axis, float angle) {
        rotation = Quatf::RotateAxis(axis[0], axis[1], axis[2], angle) * rotation;
    }

    void Translate(const Vec3f& offset) {
        position = position + offset;
    }

    void TranslateLocal(const Vec3f& offset) {
        position = position + (Right() * offset[0]);
        position = position + (Up() * offset[1]);
        position = position + (Forward() * offset[2]);
    }
};

}
