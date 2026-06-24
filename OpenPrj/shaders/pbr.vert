#version 330 core
layout (location = 0) in vec3 aPos;

uniform float scale;

layout (location = 1) in vec3 color;
layout (location = 2) in vec2 vertex;

out vec2 texCoordinate;

out vec3 theColor;
void main()
{
    gl_Position = vec4(aPos.x + aPos.x * scale, aPos.y + aPos.y * scale, aPos.z + aPos.z * scale, 1.0);
    theColor = color;
    texCoordinate = vertex;
}