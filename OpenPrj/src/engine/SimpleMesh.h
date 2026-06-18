#pragma once

#include "NKGlad/include/glad/gl.h"

namespace graphics {

class SimpleMesh {
public:
    bool CreateTriangle();
    void Draw() const;
    void Shutdown();

private:
    GLuint mVao = 0;
    GLuint mVbo = 0;
};

} // namespace graphics
