#pragma once

#include "VertexBuffer.hpp"
#include "NKGlad/include/glad/gl.h"
#include <vector>

namespace engine {

struct MeshData {
    std::vector<float>          vertices;
    std::vector<uint32_t>       indices;
    std::vector<VertexAttribute> layout;
};

class Mesh {
public:
    Mesh();
    ~Mesh();

    // No copy
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Configure mesh with vertex/index data (can be called before GL loads)
    void SetData(const MeshData& data);

    // Must be called after gladLoadGL — uploads to GPU
    void Init();

    // Issue draw call
    void Render() const;

    VertexBuffer&       GetVertexBuffer()       { return vertexBuffer; }
    const VertexBuffer& GetVertexBuffer() const { return vertexBuffer; }

    // ---- Primitive factories ----
    static MeshData CreateCube();
    static MeshData CreateSphere(uint32_t segments = 32, uint32_t rings = 16);
    static MeshData CreatePlane(float width = 1.0f, float height = 1.0f, uint32_t subdivisions = 1);
    static MeshData CreatePyramid();

private:
    VertexBuffer vertexBuffer;
};

} // namespace engine
