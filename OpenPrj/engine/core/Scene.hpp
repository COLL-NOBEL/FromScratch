#pragma once
// engine/core/Scene.hpp
// A Scene owns a list of GameObjects and drives their Update/Render loop.

#include "GameObject.hpp"
#include "../graphics/Renderer.hpp"
#include "Time.hpp"
#include <vector>
#include <memory>
#include <string>

namespace engine {

class Scene {
public:
    explicit Scene(const std::string& name = "Scene");
    virtual ~Scene() = default;

    // Called once after GL is loaded
    virtual void OnInit(Renderer& renderer) {}

    // Called every frame
    virtual void OnUpdate(float deltaTime) {}

    // Called every frame after update — use renderer to issue draw calls
    virtual void OnRender(Renderer& renderer) {}

    // Called when the scene is destroyed
    virtual void OnShutdown() {}

    // ---- GameObject management ----------------------------------------

    // Add a game object and return a raw pointer to it
    template<typename T = GameObject, typename... Args>
    T* AddObject(Args&&... args) {
        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        objects.push_back(std::move(obj));
        return ptr;
    }

    void RemoveObject(GameObject* obj);

    const std::vector<std::shared_ptr<GameObject>>& GetObjects() const { return objects; }

    const std::string& GetName() const { return name; }

    // Default render: iterate all objects and call obj->Render(renderer)
    void RenderAll(Renderer& renderer);

protected:
    std::string name;
    std::vector<std::shared_ptr<GameObject>> objects;
};

} // namespace engine
