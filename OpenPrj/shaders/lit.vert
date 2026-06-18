#version 460 core

// -------------------------------------------------------
// lit.vert  —  Phong-lit vertex shader
// Outputs world-space position, normal, and UV to frag.
// -------------------------------------------------------

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aUV;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vNormal;
out vec3 vFragPos;
out vec2 vUV;

void main()
{
    vec4 worldPos  = uModel * vec4(aPosition, 1.0);
    gl_Position    = uProjection * uView * worldPos;

    vFragPos = vec3(worldPos);
    // Normal matrix: transpose-inverse of upper-left 3x3 of model
    vNormal  = mat3(transpose(inverse(uModel))) * aNormal;
    vUV      = aUV;
}
