#version 460 core

// unlit.frag — flat colour or texture, no lighting calculation
in vec2 vUV;

uniform vec3      uAlbedo;
uniform sampler2D uAlbedoTex;
uniform bool      uUseTexture;

out vec4 FragColor;

void main()
{
    vec3 col = uAlbedo;
    if (uUseTexture) col *= texture(uAlbedoTex, vUV).rgb;
    FragColor = vec4(col, 1.0);
}
