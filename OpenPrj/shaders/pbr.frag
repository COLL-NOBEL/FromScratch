#version 330 core

out vec4 FragColor;

in vec2 texCoordinate;
in vec3 theColor;

uniform sampler2D actualTexture;

void main()
{
    FragColor = texture(actualTexture, texCoordinate);
}
