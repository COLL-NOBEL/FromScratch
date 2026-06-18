#pragma once

#include "engine/ShaderProgram.h"
#include "engine/SimpleMesh.h"

namespace graphics {

class Renderer {
public:
    bool Init(int viewportWidth, int viewportHeight);
    void Resize(int viewportWidth, int viewportHeight);
    void Render(float elapsedSeconds);
    void Shutdown();

private:
    int mViewportWidth = 1;
    int mViewportHeight = 1;

    ShaderProgram mShader;
    SimpleMesh mMesh;
};

} // namespace graphics
