#include "Texture.h"

Texture::Texture(const char* image, GLenum texType, GLenum slot, GLenum format, GLenum pixelType) 
{
    type = texType;
    int imgWidth, imgHeight, colChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* imgBytes = stbi_load(image, &imgWidth, &imgHeight, &colChannels, 0);

    glGenTextures(1, &ID);
    glActiveTexture(slot);
    glBindTexture(texType, ID); // Binding texture for loading to texture

    // More texture settings, setting the texture wrapping and filtering parameters
    glTexParameteri(texType, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(texType, 0, GL_RGB, imgWidth, imgHeight, 0, format, pixelType, imgBytes);
    glGenerateMipmap(texType);  //  Generate image mipmap to correctly map image from a distance

    // image texture has already been loaded, so we free the memory in imgBytes
    stbi_image_free(imgBytes);
    glBindTexture(texType, 0); // Unbinding texture to avoid unnecessary modification
}

void Texture::texUnit(Shader shader, const char* uniform, GLuint unit)
{
    GLuint textureUniform = glGetUniformLocation(shader.ID, uniform); // Declaring Uniform for texturing vertices on texture

    shader.Activate();  // Activating the shader program for rendering
    glUniform1i(textureUniform, unit);
}

void Texture::Bind()
{
    glBindTexture(type, ID);
}

void Texture::Unbind()
{
    glBindTexture(type, 0);
}

void Texture::Delete()
{
    glDeleteTextures(1, &ID);
}