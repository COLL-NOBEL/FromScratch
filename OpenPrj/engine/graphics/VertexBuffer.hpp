#pragma once

#include "NKGlad/include/glad/gl.h"
#include <vector>
#include <cstdint>

namespace engine {

enum class VertexAttributeType {
    Float,
    Vec2,
    Vec3,
    Vec4,
    Mat4,
};

struct VertexAttribute {
    VertexAttributeType type;
    GLuint index;
    GLuint offset;

    GLint GetComponentCount() const {
        switch (type) {
            case VertexAttributeType::Float: return 1;
            case VertexAttributeType::Vec2:  return 2;
            case VertexAttributeType::Vec3:  return 3;
            case VertexAttributeType::Vec4:  return 4;
            case VertexAttributeType::Mat4:  return 16;
            default: return 0;
        }
    }

    GLuint GetByteSize() const {
        return (GLuint)(GetComponentCount() * sizeof(float));
    }
};

class VertexBuffer {
public:
    VertexBuffer();
    ~VertexBuffer();

    // No copy
    VertexBuffer(const VertexBuffer&) = delete;
    VertexBuffer& operator=(const VertexBuffer&) = delete;

    // Move
    VertexBuffer(VertexBuffer&& other) noexcept;
    VertexBuffer& operator=(VertexBuffer&& other) noexcept;

    // Call after OpenGL is loaded
    void Init();

    void SetLayout(const std::vector<VertexAttribute>& attrs);
    GLuint GetStride() const { return stride; }

    // vertexCount = number of vertices (not number of floats)
    void SetData(const void* data, size_t vertexCount);
    void SetIndexData(const uint32_t* indices, size_t indexCount);

    void Bind()   const;
    void Unbind() const;

    GLuint GetVAO()        const { return vao; }
    GLuint GetVBO()        const { return vbo; }
    GLuint GetIBO()        const { return ibo; }
    size_t GetIndexCount() const { return indexCount; }
    size_t GetVertexCount()const { return vertexCount; }

    bool IsInitialized()   const { return initialized; }

private:
    void EnsureInit();

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ibo = 0;
    GLuint stride = 0;
    size_t vertexCount = 0;
    size_t indexCount  = 0;
    bool   initialized = false;
    std::vector<VertexAttribute> layout;

    // Pending data stored until Init() is called
    std::vector<uint8_t>  pendingVertexData;
    std::vector<uint32_t> pendingIndexData;
    bool hasPendingVertex = false;
    bool hasPendingIndex  = false;
};

} // namespace engine
