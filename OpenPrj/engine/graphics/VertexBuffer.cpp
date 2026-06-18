#include "VertexBuffer.hpp"
#include "NKLogger/NkLog.h"
#include <cstring>

namespace engine {

using namespace nkentseu;

// ---- Constructor / Destructor -------------------------------------------

VertexBuffer::VertexBuffer()
    : vao(0), vbo(0), ibo(0), stride(0),
      vertexCount(0), indexCount(0), initialized(false) {}

VertexBuffer::~VertexBuffer() {
    if (ibo != 0) glDeleteBuffers(1, &ibo);
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
}

// ---- Move semantics ------------------------------------------------------

VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ibo(other.ibo),
      stride(other.stride), vertexCount(other.vertexCount),
      indexCount(other.indexCount), initialized(other.initialized),
      layout(std::move(other.layout)),
      pendingVertexData(std::move(other.pendingVertexData)),
      pendingIndexData(std::move(other.pendingIndexData)),
      hasPendingVertex(other.hasPendingVertex),
      hasPendingIndex(other.hasPendingIndex)
{
    other.vao = other.vbo = other.ibo = 0;
    other.initialized = false;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept {
    if (this != &other) {
        if (ibo != 0) glDeleteBuffers(1, &ibo);
        if (vbo != 0) glDeleteBuffers(1, &vbo);
        if (vao != 0) glDeleteVertexArrays(1, &vao);

        vao = other.vao; vbo = other.vbo; ibo = other.ibo;
        stride = other.stride; vertexCount = other.vertexCount;
        indexCount = other.indexCount; initialized = other.initialized;
        layout = std::move(other.layout);
        pendingVertexData = std::move(other.pendingVertexData);
        pendingIndexData  = std::move(other.pendingIndexData);
        hasPendingVertex  = other.hasPendingVertex;
        hasPendingIndex   = other.hasPendingIndex;

        other.vao = other.vbo = other.ibo = 0;
        other.initialized = false;
    }
    return *this;
}

// ---- Initialization (call AFTER gladLoadGL) ----------------------------

void VertexBuffer::Init() {
    if (initialized) return;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    initialized = true;

    // Apply pending layout
    if (!layout.empty()) {
        Bind();
        GLuint offset = 0;
        for (const auto& attr : layout) {
            glEnableVertexAttribArray(attr.index);
            glVertexAttribPointer(attr.index, attr.GetComponentCount(),
                                  GL_FLOAT, GL_FALSE, stride,
                                  (void*)(uintptr_t)offset);
            offset += attr.GetByteSize();
        }
        Unbind();
    }

    // Upload pending vertex data
    if (hasPendingVertex && !pendingVertexData.empty()) {
        Bind();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)pendingVertexData.size(),
                     pendingVertexData.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        Unbind();
        pendingVertexData.clear();
        hasPendingVertex = false;
    }

    // Upload pending index data
    if (hasPendingIndex && !pendingIndexData.empty()) {
        Bind();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)(pendingIndexData.size() * sizeof(uint32_t)),
                     pendingIndexData.data(), GL_STATIC_DRAW);
        Unbind();
        pendingIndexData.clear();
        hasPendingIndex = false;
    }

    logger.Info("[VertexBuffer] Initialized VAO/VBO/IBO\n");
}

// ---- Private helper ------------------------------------------------------

void VertexBuffer::EnsureInit() {
    if (!initialized) Init();
}

// ---- Layout / data upload -----------------------------------------------

void VertexBuffer::SetLayout(const std::vector<VertexAttribute>& attrs) {
    layout = attrs;
    stride = 0;
    for (const auto& attr : layout) {
        stride += attr.GetByteSize();
    }

    if (!initialized) return;   // will be applied in Init()

    Bind();
    GLuint offset = 0;
    for (const auto& attr : layout) {
        glEnableVertexAttribArray(attr.index);
        glVertexAttribPointer(attr.index, attr.GetComponentCount(),
                              GL_FLOAT, GL_FALSE, stride,
                              (void*)(uintptr_t)offset);
        offset += attr.GetByteSize();
    }
    Unbind();
}

void VertexBuffer::SetData(const void* data, size_t vcount) {
    vertexCount = vcount;
    size_t byteSize = vcount * stride;

    if (!initialized) {
        // Cache for later
        pendingVertexData.resize(byteSize);
        std::memcpy(pendingVertexData.data(), data, byteSize);
        hasPendingVertex = true;
        return;
    }

    Bind();
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)byteSize, data, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    Unbind();

    logger.Infof("[VertexBuffer] Uploaded %zu vertices (%zu bytes)\n", vertexCount, byteSize);
}

void VertexBuffer::SetIndexData(const uint32_t* indices, size_t icount) {
    indexCount = icount;

    if (!initialized) {
        pendingIndexData.assign(indices, indices + icount);
        hasPendingIndex = true;
        return;
    }

    Bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 (GLsizeiptr)(icount * sizeof(uint32_t)),
                 indices, GL_STATIC_DRAW);
    Unbind();

    logger.Infof("[VertexBuffer] Uploaded %zu indices\n", indexCount);
}

// ---- Bind / Unbind -------------------------------------------------------

void VertexBuffer::Bind() const {
    glBindVertexArray(vao);
}

void VertexBuffer::Unbind() const {
    glBindVertexArray(0);
}

} // namespace engine
