// engine/core/GameObject.cpp

#include "GameObject.hpp"
#include "../graphics/Renderer.hpp"
#include "../graphics/Mesh.hpp"
#include "../graphics/Material.hpp"

namespace engine {

GameObject::GameObject()
    : transform(), material(nullptr), mesh(nullptr), isActive(true) {}

GameObject::~GameObject() {}

void GameObject::Render(Renderer* renderer) {
    if (!renderer || !mesh || !material) return;

    // Use a shared_ptr alias — renderer expects shared_ptr but we hold raw pointers
    // wrapped here via aliased shared_ptrs with no-op deleters.
    auto meshPtr     = std::shared_ptr<Mesh>(mesh,     [](Mesh*){});
    auto materialPtr = std::shared_ptr<Material>(material, [](Material*){});

    renderer->RenderMesh(meshPtr, materialPtr, transform);
}

} // namespace engine
