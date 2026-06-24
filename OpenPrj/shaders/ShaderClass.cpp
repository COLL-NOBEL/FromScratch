#include "ShaderClass.h"

std::string getFileContent(const char* fileName) 
{
    std::ifstream in(fileName, std::ios::binary);
    if (in) {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }
    throw(errno);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile) 
{
    std::string vertexCode = getFileContent(vertexFile);  // Loading vertex shader to code
    std::string fragmentCode = getFileContent(fragmentFile);  // Loading fragment shader to code

    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();   // Converting string to C-Type string for Opengl

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);      // Creating pointer to a vertex shader  OpenGL object
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);  // Filing the memory of the vertex shader with the actual shader code
    glCompileShader(vertexShader);  // compiling shader code into the vertex shader OpenGL object

    GLuint fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    ID = glCreateProgram();

    glAttachShader(ID, vertexShader);    // Attaching the compiled vertex shader to the shader program
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);        // Linking the shader program to make it ready for use in rendering
    glDeleteShader(vertexShader);        // Deleting the vertex shader as it is no longer needed after linking
    glDeleteShader(fragmentShader);
}

void Shader::Activate() 
{
    glUseProgram(ID);     // Activating the shader program for rendering
}

void Shader::Delete()
{
    glDeleteProgram(ID);        // Deleting the shader program to free up GPU memory
}