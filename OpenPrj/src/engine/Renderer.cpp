#include "engine/Renderer.h"

#include "NKMath/NKMath.h"

namespace graphics {

bool Renderer::Init(int viewportWidth, int viewportHeight) {
    Resize(viewportWidth, viewportHeight);

    if (!mShader.LoadFromFiles("OpenPrj/shaders/basic.vert", "OpenPrj/shaders/basic.frag")) {
        return false;
    }

    if (!mMesh.CreateTriangle()) {
        mShader.Shutdown();
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.10f, 0.16f, 1.0f);

    return true;
}

void Renderer::Resize(int viewportWidth, int viewportHeight) {
    mViewportWidth = (viewportWidth > 0) ? viewportWidth : 1;
    mViewportHeight = (viewportHeight > 0) ? viewportHeight : 1;
}

void Renderer::Render(float elapsedSeconds) {
    glViewport(0, 0, mViewportWidth, mViewportHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mShader.Bind();

    using namespace nkentseu::math;

    const float aspect = static_cast<float>(mViewportWidth) / static_cast<float>(mViewportHeight);

    const NkVec3f eye(0.0f, 0.0f, 2.25f);
    const NkVec3f center(0.0f, 0.0f, 0.0f);
    const NkVec3f up(0.0f, 1.0f, 0.0f);

    const NkMat4f model = NkMat4f::RotationY(NkAngle(elapsedSeconds * 45.0f));
    const NkMat4f view = NkMat4f::LookAt(eye, center, up);
    const NkMat4f projection = NkMat4f::Perspective(NkAngle(60.0f), aspect, 0.1f, 100.0f);
    const NkMat4f mvp = projection * view * model;

    const GLint uMvp = mShader.FindUniform("uMVP");
    if (uMvp >= 0) {
        glUniformMatrix4fv(uMvp, 1, GL_FALSE, mvp.data);
    }

    mMesh.Draw();
}

void Renderer::Shutdown() {
    mMesh.Shutdown();
    mShader.Shutdown();
}

} // namespace graphics
