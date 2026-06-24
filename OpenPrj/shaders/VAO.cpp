#include "VAO.h"

VAO::VAO() 
{
    // Generating a Vertex Array Object (VAO) to store the vertex attribute configuration
    glGenVertexArrays(1, &ID);
}

void VAO::LinkAttributes(VBO& VBO, GLuint layout, GLuint numComponent, GLenum type, GLsizeiptr stride, void* offset) 
{
    // Binding the VBO to the GL_ARRAY_BUFFER target to specify that it will be used for vertex attribute data
    VBO.Bind();
    // Setting up the vertex attribute pointer for the specified attribute index
    glVertexAttribPointer(layout, numComponent, type, GL_FALSE, stride, offset);
    // Enabling the vertex attribute array for the specified attribute index
    glEnableVertexAttribArray(layout);
    // Unbinding the VBO to prevent accidental modification of the buffer data
    VBO.Unbind();
}

void VAO::Bind()
{
    glBindVertexArray(ID);        // Binding the VAO to make it the current vertex array object
}

void VAO::Unbind ()
{
    glBindVertexArray(0);      // Unbinding the VAO to prevent accidental modification of the vertex attribute configuration
}

void VAO::Delete()
{
    glDeleteVertexArrays(1, &ID);    // Deleting the VAO to free up GPU memory
}