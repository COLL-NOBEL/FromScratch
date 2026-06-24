#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include "NKGlad/include/glad/gl.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cerrno>

std::string getFileContent(const char* fileName);

class Shader 
{
public: 
    GLuint ID;

    Shader(const char* vertexFile, const char* fragmentFile);

    void Activate();
    void Delete();

private:
    void compileErrors(unsigned int shader, const char* type);
};

#endif