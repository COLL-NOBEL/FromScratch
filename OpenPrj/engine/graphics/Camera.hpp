#pragma once

#include "../core/Math.hpp"

namespace engine {

class Camera {
public:
    Camera();
    virtual ~Camera() = default;

    Mat4f GetViewMatrix() const;
    Mat4f GetProjectionMatrix() const;

    void SetPosition(const Vec3f& pos) { position = pos; }
    void SetRotation(const Quatf& rot) { rotation = rot; }
    void SetLookAt(const Vec3f& target, const Vec3f& up = Vec3f(0, 1, 0));

    const Vec3f& GetPosition() const { return position; }
    const Quatf& GetRotation() const { return rotation; }

    void SetPerspective(float fov, float aspect, float near, float far);
    void SetOrthographic(float left, float right, float bottom, float top, float near, float far);

    Vec3f Forward() const;
    Vec3f Right() const;
    Vec3f Up() const;

    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);

    void Rotate(float yaw, float pitch, float roll);

protected:
    Vec3f position;
    Quatf rotation;

    Mat4f projectionMatrix;
    bool isDirty = true;

    float fov = 0.785398f;
    float aspect = 1.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    bool isPerspective = true;
};

}
