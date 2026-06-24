#include "VBO.h"

VBO::VBO(GLfloat* vertices, GLsizeiptr size) 
{
    glGenBuffers(1, &ID);         // Generating a Vertex Buffer Object (VBO) to store the vertex data
    glBindBuffer(GL_ARRAY_BUFFER, ID); // Bind the buffer object to the GL_ARRAY_BUFFER target

    // Uploading the index data to the GPU by copying the data from the CPU memory to the GPU memory
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
}

void VBO::Bind()
{
    glBindBuffer(GL_ARRAY_BUFFER, ID); // Bind the buffer object to the GL_ARRAY_BUFFER target
}

void VBO::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);   // Unbinding the VBO to prevent accidental modification of the buffer data
}

void VBO::Delete()
{
    glDeleteBuffers(1, &ID);      // Deleting the VBO to free up GPU memory
}