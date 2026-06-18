#version 460 core

// unlit.vert — pass position + UV, no lighting
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;   // unused but keeps layout consistent
layout (location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vUV;

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(aPosition, 1.0);
    vUV = aUV;
}
