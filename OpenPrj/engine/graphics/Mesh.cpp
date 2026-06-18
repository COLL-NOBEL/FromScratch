#include "Mesh.hpp"
#include "NKLogger/NkLog.h"
#include <cmath>

namespace engine {

using namespace nkentseu;

Mesh::Mesh() {}

Mesh::~Mesh() {}

// vertexData is a flat float array; floatsPerVertex is computed from the layout
void Mesh::SetData(const MeshData& data) {
    vertexBuffer.SetLayout(data.layout);

    // Compute floats-per-vertex from layout
    size_t floatsPerVertex = 0;
    for (const auto& attr : data.layout)
        floatsPerVertex += (size_t)attr.GetComponentCount();

    size_t numVertices = (floatsPerVertex > 0)
        ? data.vertices.size() / floatsPerVertex
        : 0;

    vertexBuffer.SetData(data.vertices.data(), numVertices);

    if (!data.indices.empty()) {
        vertexBuffer.SetIndexData(data.indices.data(), data.indices.size());
    }
}

void Mesh::Init() {
    vertexBuffer.Init();
}

void Mesh::Render() const {
    vertexBuffer.Bind();
    if (vertexBuffer.GetIndexCount() > 0) {
        glDrawElements(GL_TRIANGLES, (GLsizei)vertexBuffer.GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertexBuffer.GetVertexCount());
    }
    vertexBuffer.Unbind();
}

// ============================================================
// Primitive factories
// Each vertex: position (3 floats) + normal (3 floats) + UV (2 floats)
//              = 8 floats per vertex
// Layout attrs: Vec3 @ loc 0, Vec3 @ loc 1, Vec2 @ loc 2
// ============================================================

static std::vector<VertexAttribute> StandardLayout() {
    return {
        {VertexAttributeType::Vec3, 0, 0 },  // position
        {VertexAttributeType::Vec3, 1, 12},  // normal
        {VertexAttributeType::Vec2, 2, 24},  // uv
    };
}

MeshData Mesh::CreateCube() {
    MeshData data;
    data.layout = StandardLayout();

    // 24 vertices (4 per face x 6 faces)
    // Each vertex: px py pz  nx ny nz  u v
    float verts[] = {
        // Back face (z = -0.5, normal 0,0,-1)
        -0.5f,-0.5f,-0.5f,  0, 0,-1,  0,0,
         0.5f,-0.5f,-0.5f,  0, 0,-1,  1,0,
         0.5f, 0.5f,-0.5f,  0, 0,-1,  1,1,
        -0.5f, 0.5f,-0.5f,  0, 0,-1,  0,1,
        // Front face (z = +0.5, normal 0,0,1)
        -0.5f,-0.5f, 0.5f,  0, 0, 1,  0,0,
         0.5f,-0.5f, 0.5f,  0, 0, 1,  1,0,
         0.5f, 0.5f, 0.5f,  0, 0, 1,  1,1,
        -0.5f, 0.5f, 0.5f,  0, 0, 1,  0,1,
        // Left face (x = -0.5, normal -1,0,0)
        -0.5f, 0.5f, 0.5f, -1, 0, 0,  1,0,
        -0.5f, 0.5f,-0.5f, -1, 0, 0,  1,1,
        -0.5f,-0.5f,-0.5f, -1, 0, 0,  0,1,
        -0.5f,-0.5f, 0.5f, -1, 0, 0,  0,0,
        // Right face (x = +0.5, normal 1,0,0)
         0.5f, 0.5f, 0.5f,  1, 0, 0,  1,0,
         0.5f, 0.5f,-0.5f,  1, 0, 0,  1,1,
         0.5f,-0.5f,-0.5f,  1, 0, 0,  0,1,
         0.5f,-0.5f, 0.5f,  1, 0, 0,  0,0,
        // Bottom face (y = -0.5, normal 0,-1,0)
        -0.5f,-0.5f,-0.5f,  0,-1, 0,  0,1,
         0.5f,-0.5f,-0.5f,  0,-1, 0,  1,1,
         0.5f,-0.5f, 0.5f,  0,-1, 0,  1,0,
        -0.5f,-0.5f, 0.5f,  0,-1, 0,  0,0,
        // Top face (y = +0.5, normal 0,1,0)
        -0.5f, 0.5f,-0.5f,  0, 1, 0,  0,1,
         0.5f, 0.5f,-0.5f,  0, 1, 0,  1,1,
         0.5f, 0.5f, 0.5f,  0, 1, 0,  1,0,
        -0.5f, 0.5f, 0.5f,  0, 1, 0,  0,0,
    };

    uint32_t idx[] = {
         0, 1, 2,  2, 3, 0,    // back
         4, 5, 6,  6, 7, 4,    // front
         8, 9,10, 10,11, 8,    // left
        12,13,14, 14,15,12,    // right
        16,17,18, 18,19,16,    // bottom
        20,21,22, 22,23,20,    // top
    };

    data.vertices.assign(verts, verts + 24*8);
    data.indices.assign(idx, idx + 36);
    logger.Info("[Mesh] Created cube\n");
    return data;
}

MeshData Mesh::CreateSphere(uint32_t segments, uint32_t rings) {
    MeshData data;
    data.layout = StandardLayout();

    const float pi = 3.14159265f;

    for (uint32_t r = 0; r <= rings; r++) {
        float phi    = pi * r / rings;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        float v_uv   = (float)r / rings;

        for (uint32_t s = 0; s <= segments; s++) {
            float theta    = 2.0f * pi * s / segments;
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            float u_uv     = (float)s / segments;

            float nx = cosTheta * sinPhi;
            float ny = cosPhi;
            float nz = sinTheta * sinPhi;

            // position (radius = 0.5)
            data.vertices.push_back(nx * 0.5f);
            data.vertices.push_back(ny * 0.5f);
            data.vertices.push_back(nz * 0.5f);
            // normal
            data.vertices.push_back(nx);
            data.vertices.push_back(ny);
            data.vertices.push_back(nz);
            // uv
            data.vertices.push_back(u_uv);
            data.vertices.push_back(v_uv);
        }
    }

    for (uint32_t r = 0; r < rings; r++) {
        for (uint32_t s = 0; s < segments; s++) {
            uint32_t a = r * (segments + 1) + s;
            uint32_t b = a + segments + 1;
            data.indices.push_back(a);
            data.indices.push_back(b);
            data.indices.push_back(a + 1);
            data.indices.push_back(a + 1);
            data.indices.push_back(b);
            data.indices.push_back(b + 1);
        }
    }

    logger.Infof("[Mesh] Created sphere (%u segments, %u rings)\n", segments, rings);
    return data;
}

MeshData Mesh::CreatePlane(float width, float height, uint32_t subdivisions) {
    MeshData data;
    data.layout = StandardLayout();

    uint32_t divs   = subdivisions + 1;
    float step_x    = width  / subdivisions;
    float step_y    = height / subdivisions;

    for (uint32_t y = 0; y <= subdivisions; y++) {
        for (uint32_t x = 0; x <= subdivisions; x++) {
            float px = -width  * 0.5f + x * step_x;
            float pz = -height * 0.5f + y * step_y;
            float u  = (float)x / subdivisions;
            float v  = (float)y / subdivisions;

            data.vertices.push_back(px);  data.vertices.push_back(0.0f); data.vertices.push_back(pz);
            data.vertices.push_back(0.0f);data.vertices.push_back(1.0f); data.vertices.push_back(0.0f);
            data.vertices.push_back(u);   data.vertices.push_back(v);
        }
    }

    for (uint32_t y = 0; y < subdivisions; y++) {
        for (uint32_t x = 0; x < subdivisions; x++) {
            uint32_t a = y * divs + x;
            uint32_t b = a + divs;
            data.indices.push_back(a);   data.indices.push_back(b);   data.indices.push_back(a+1);
            data.indices.push_back(a+1); data.indices.push_back(b);   data.indices.push_back(b+1);
        }
    }

    logger.Infof("[Mesh] Created plane (%.1f x %.1f)\n", width, height);
    return data;
}

MeshData Mesh::CreatePyramid() {
    MeshData data;
    data.layout = StandardLayout();

    // Apex at top, square base
    float verts[] = {
        // Front face
         0.0f, 0.5f, 0.0f,   0, 0.894f, 0.447f,   0.5f, 1.0f,
        -0.5f,-0.5f, 0.5f,   0, 0.894f, 0.447f,   0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,   0, 0.894f, 0.447f,   1.0f, 0.0f,
        // Back face
         0.0f, 0.5f, 0.0f,   0, 0.894f,-0.447f,   0.5f, 1.0f,
         0.5f,-0.5f,-0.5f,   0, 0.894f,-0.447f,   0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,   0, 0.894f,-0.447f,   1.0f, 0.0f,
        // Right face
         0.0f, 0.5f, 0.0f,   0.894f, 0.447f, 0,   0.5f, 1.0f,
         0.5f,-0.5f, 0.5f,   0.894f, 0.447f, 0,   0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,   0.894f, 0.447f, 0,   1.0f, 0.0f,
        // Left face
         0.0f, 0.5f, 0.0f,  -0.894f, 0.447f, 0,   0.5f, 1.0f,
        -0.5f,-0.5f,-0.5f,  -0.894f, 0.447f, 0,   0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  -0.894f, 0.447f, 0,   1.0f, 0.0f,
        // Base (2 triangles)
        -0.5f,-0.5f,-0.5f,   0,-1,0,  0,1,
         0.5f,-0.5f,-0.5f,   0,-1,0,  1,1,
         0.5f,-0.5f, 0.5f,   0,-1,0,  1,0,
        -0.5f,-0.5f, 0.5f,   0,-1,0,  0,0,
    };

    uint32_t idx[] = {
        0,1,2,   3,4,5,   6,7,8,   9,10,11,
        12,13,14, 14,15,12
    };

    data.vertices.assign(verts, verts + 16*8);
    data.indices.assign(idx, idx + 18);
    logger.Info("[Mesh] Created pyramid\n");
    return data;
}

} // namespace engine
