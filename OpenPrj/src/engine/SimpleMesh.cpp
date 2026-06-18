#include "engine/SimpleMesh.h"

namespace graphics {

bool SimpleMesh::CreateTriangle() {
    Shutdown();

    constexpr float kTriangleVertices[] = {
        // Position xyz      // Color rgb
         0.0f,  0.6f, 0.0f,  1.0f, 0.3f, 0.3f,
        -0.6f, -0.4f, 0.0f,  0.3f, 1.0f, 0.3f,
         0.6f, -0.4f, 0.0f,  0.3f, 0.5f, 1.0f,
    };

    glGenVertexArrays(1, &mVao);
    glGenBuffers(1, &mVbo);

    if (mVao == 0 || mVbo == 0) {
        Shutdown();
        return false;
    }

    glBindVertexArray(mVao);

    glBindBuffer(GL_ARRAY_BUFFER, mVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(kTriangleVertices), kTriangleVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * static_cast<GLsizei>(sizeof(float)),
        reinterpret_cast<void*>(0)
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * static_cast<GLsizei>(sizeof(float)),
        reinterpret_cast<void*>(3 * sizeof(float))
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

void SimpleMesh::Draw() const {
    if (mVao == 0) {
        return;
    }

    glBindVertexArray(mVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void SimpleMesh::Shutdown() {
    if (mVbo != 0) {
        glDeleteBuffers(1, &mVbo);
        mVbo = 0;
    }

    if (mVao != 0) {
        glDeleteVertexArrays(1, &mVao);
        mVao = 0;
    }
}

} // namespace graphics
