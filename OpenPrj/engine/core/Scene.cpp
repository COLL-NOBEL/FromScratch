// engine/core/Scene.cpp

#include "Scene.hpp"
#include <algorithm>

namespace engine {

Scene::Scene(const std::string& n) : name(n) {}

void Scene::RemoveObject(GameObject* obj) {
    objects.erase(
        std::remove_if(objects.begin(), objects.end(),
            [obj](const std::shared_ptr<GameObject>& p){ return p.get() == obj; }),
        objects.end()
    );
}

void Scene::RenderAll(Renderer& renderer) {
    for (auto& obj : objects) {
        if (obj && obj->isActive) {
            obj->Render(&renderer);
        }
    }
}

} // namespace engine
