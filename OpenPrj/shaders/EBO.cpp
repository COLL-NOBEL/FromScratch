#include "EBO.h"

EBO::EBO(GLuint* indices, GLsizeiptr size) 
{
    glGenBuffers(1, &ID);         // Generating a Element Buffer Object (EBO) to store the index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); // Bind the buffer object to the GL_ELEMENT_ARRAY_BUFFER target

    // Uploading the index data to the GPU by copying the data from the CPU memory to the GPU memory
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}

void EBO::Bind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID); // Bind the buffer object to the GL_ELEMENT_ARRAY_BUFFER target
}

void EBO::Unbind()
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);   // Unbinding the EBO to prevent accidental modification of the buffer data
}

void EBO::Delete()
{
    glDeleteBuffers(1, &ID);      // Deleting the EBO to free up GPU memory
}