#pragma once
// engine/core/Application.hpp
// Base class for the engine application layer.
// Inherit from this and override OnInit / OnUpdate / OnRender / OnShutdown.

#include "Scene.hpp"
#include "Time.hpp"
#include "Input.hpp"
#include "../graphics/Renderer.hpp"

#include "NKWindow/NkWindow.h"
#include "NKContext/Factory/NkContextFactory.h"
#include "NKContext/Core/NkContextDesc.h"
#include "NKContext/Core/NkNativeContextAccess.h"
#include "NKContext/Core/NkOpenGLDesc.h"
#include "NKEvent/NkEvent.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKEvent/NkEventSystem.h"
#include "NKLogger/NkLog.h"

#include "NKGlad/include/glad/wgl.h"
#include "NKGlad/include/glad/gl.h"

#if defined(Bool)
    #undef Bool
#endif

#include <memory>
#include <string>

namespace engine {

struct AppConfig {
    std::string title   = "Engine";
    uint32_t    width   = 1280;
    uint32_t    height  = 720;
    bool        vsync   = true;
    bool        resizable = true;
    int         glMajor = 4;
    int         glMinor = 6;
};

class Application {
public:
    explicit Application(const AppConfig& cfg = {});
    virtual ~Application();

    // Entry point — creates window, loads GL, runs loop
    int Run();

    // ---- Override these in your subclass ----------------------------
    virtual void OnInit()              {}
    virtual void OnUpdate(float dt)    {}
    virtual void OnRender()            {}
    virtual void OnShutdown()          {}
    virtual void OnResize(uint32_t w, uint32_t h) {}

    // ---- Accessors --------------------------------------------------
    Renderer&   GetRenderer()          { return renderer; }
    Time&        GetTime()             { return time; }
    uint32_t     GetWidth()  const     { return width; }
    uint32_t     GetHeight() const     { return height; }

    // Push an active scene (OnInit called immediately if GL loaded)
    void SetScene(std::shared_ptr<Scene> scene);
    Scene* GetScene() const { return activeScene.get(); }

protected:
    AppConfig cfg;
    Renderer  renderer;
    Time      time;

    uint32_t  width  = 0;
    uint32_t  height = 0;

    std::shared_ptr<Scene>           activeScene;
    nkentseu::NkWindow               window;
    nkentseu::NkIGraphicsContext*    ctx = nullptr;

private:
    bool InitWindow();
    bool LoadGL();
    void MainLoop();
    void Shutdown();

    bool running = true;
};

} // namespace engine
