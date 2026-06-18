#include "Camera.hpp"

namespace engine {

Camera::Camera()
    : position(0, 0, 5), rotation() {
    projectionMatrix = Transform::Perspective(fov, aspect, nearPlane, farPlane);
}

Mat4f Camera::GetViewMatrix() const {
    Vec3f forward = Forward();
    Vec3f target = position + forward;
    return Transform::LookAt(position, target, Up());
}

Mat4f Camera::GetProjectionMatrix() const {
    return projectionMatrix;
}

void Camera::SetLookAt(const Vec3f& target, const Vec3f& up) {
    Vec3f direction = (target - position).Normalize();
    Vec3f right = direction.CrossProduct(up).Normalize();
    Vec3f actualUp = right.CrossProduct(direction).Normalize();
}

void Camera::SetPerspective(float f, float a, float n, float f_) {
    fov = f;
    aspect = a;
    nearPlane = n;
    farPlane = f_;
    isPerspective = true;
    projectionMatrix = Transform::Perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::SetOrthographic(float left, float right, float bottom, float top, float near, float far) {
    isPerspective = false;
    nearPlane = near;
    farPlane = far;
    projectionMatrix = Transform::Orthographic(left, right, bottom, top, near, far);
}

Vec3f Camera::Forward() const {
    return rotation.Forward().Normalize();
}

Vec3f Camera::Right() const {
    return rotation.Right().Normalize();
}

Vec3f Camera::Up() const {
    return rotation.Up().Normalize();
}

void Camera::MoveForward(float distance) {
    position = position + Forward() * distance;
}

void Camera::MoveRight(float distance) {
    position = position + Right() * distance;
}

void Camera::MoveUp(float distance) {
    position = position + Up() * distance;
}

void Camera::Rotate(float yaw, float pitch, float roll) {
    Quatf yawQuat = Quatf::RotateY(yaw);
    Quatf pitchQuat = Quatf::RotateX(pitch);
    Quatf rollQuat = Quatf::RotateZ(roll);
    rotation = yawQuat * pitchQuat * rollQuat;
}

}
