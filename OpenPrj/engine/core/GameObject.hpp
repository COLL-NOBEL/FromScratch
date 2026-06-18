#pragma once

#include "Transform.hpp"

namespace engine {

class Renderer;
class Material;

class GameObject {
public:
    GameObject();
    virtual ~GameObject();

    Transform& GetTransform() { return transform; }
    const Transform& GetTransform() const { return transform; }

    void SetMaterial(class Material* mat) { material = mat; }
    class Material* GetMaterial() { return material; }
    const class Material* GetMaterial() const { return material; }

    void SetMesh(class Mesh* m) { mesh = m; }
    class Mesh* GetMesh() { return mesh; }
    const class Mesh* GetMesh() const { return mesh; }

    virtual void Update(float deltaTime) {}
    virtual void Render(class Renderer* renderer);

    bool isActive = true;

protected:
    Transform transform;
    class Material* material = nullptr;
    class Mesh* mesh = nullptr;
};

}
