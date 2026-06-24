#include "ShaderClass.h"
#include <string>

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
    compileErrors(vertexShader, "VERTEX");



    GLuint fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);
    compileErrors(fragmentShader, "FRAGMENT");

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

void Shader::compileErrors(unsigned int shader, const char* type) 
{
    GLint hasCompiled;
    char infoLog[1024];
    
    std::string typeStr(type);  // Convert to std::string
    
    if (typeStr != "PROGRAM") {  // Now compares strings properly
        glGetShaderiv(shader, GL_COMPILE_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "SHADER_COMPILATION_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &hasCompiled);
        if (hasCompiled == GL_FALSE) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cout << "SHADER_LINKING_ERROR for: " << type << "\n" << infoLog << std::endl;
        }
    }
}