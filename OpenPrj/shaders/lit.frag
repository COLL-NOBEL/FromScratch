#version 460 core

// -------------------------------------------------------
// lit.frag  —  Blinn-Phong fragment shader
// Supports up to 4 lights (directional, point, spot).
// -------------------------------------------------------

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vUV;

// ---- Material uniforms ----
uniform vec3      uAlbedo;        // base colour (RGB)
uniform float     uShininess;     // specular exponent
uniform sampler2D uAlbedoTex;     // optional texture
uniform bool      uUseTexture;    // whether to sample texture
uniform vec3      uCamPos;        // world-space camera position

// ---- Light array (max 4) ----
#define MAX_LIGHTS 4
struct LightData {
    int   type;         // 0=directional, 1=point, 2=spot
    vec3  position;
    vec3  direction;
    vec3  color;
    float intensity;
    float range;
};
uniform LightData uLights[MAX_LIGHTS];
uniform int       uLightCount;

out vec4 FragColor;

// ---- Blinn-Phong for one light ----
vec3 CalcLight(LightData L, vec3 norm, vec3 fragPos, vec3 viewDir, vec3 albedo)
{
    vec3 lightDir;
    float attenuation = 1.0;

    if (L.type == 0) {
        // Directional
        lightDir = normalize(-L.direction);
    } else {
        // Point / Spot
        vec3 toLight = L.position - fragPos;
        float dist   = length(toLight);
        lightDir     = toLight / dist;
        // Inverse-square attenuation clamped at range
        attenuation  = clamp(1.0 - (dist / L.range), 0.0, 1.0);
        attenuation *= attenuation;
    }

    // Ambient
    vec3 ambient = 0.08 * L.color * albedo;

    // Diffuse
    float diff   = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * L.color * albedo;

    // Specular (Blinn-Phong half-vector)
    vec3 halfDir  = normalize(lightDir + viewDir);
    float spec    = pow(max(dot(norm, halfDir), 0.0), uShininess);
    vec3 specular = spec * L.color * 0.4;

    return (ambient + (diffuse + specular) * attenuation) * L.intensity;
}

void main()
{
    vec3 norm    = normalize(vNormal);
    vec3 viewDir = normalize(uCamPos - vFragPos);

    vec3 albedo = uAlbedo;
    if (uUseTexture) albedo *= texture(uAlbedoTex, vUV).rgb;

    vec3 result = vec3(0.0);
    for (int i = 0; i < uLightCount && i < MAX_LIGHTS; i++) {
        result += CalcLight(uLights[i], norm, vFragPos, viewDir, albedo);
    }

    // Tone-map with simple Reinhard
    result = result / (result + vec3(1.0));

    // Gamma correct (linear -> sRGB)
    result = pow(result, vec3(1.0 / 2.2));

    FragColor = vec4(result, 1.0);
}
