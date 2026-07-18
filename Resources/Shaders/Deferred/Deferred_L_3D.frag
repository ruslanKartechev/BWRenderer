#version 450 core

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform sampler2D u_gAlbedoSmoothness;
layout(binding = 1) uniform sampler2D u_gNormal;
layout(binding = 2) uniform sampler2D u_gDepth;
layout(binding = 3) uniform samplerCube u_Skybox;

in vec2 v_uv;

struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

uniform Light DIRECTIONAL_LIGHT;
uniform vec3 VIEW_POS;
uniform mat4 MATRIX_INVERSE_PROJECTION;
uniform mat4 MATRIX_INVERSE_VIEW;
uniform float _AMBIENT_LIGHT_INTENSITY;

vec3 ReconstructWorldPos(vec2 uv, float depth) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = MATRIX_INVERSE_PROJECTION * ndc;
    viewPos /= viewPos.w;
    vec4 worldPos = MATRIX_INVERSE_VIEW * viewPos;
    return worldPos.xyz;
}


void main(){
    float depth = texture(u_gDepth, v_uv).r;
    if (depth >= 0.9999) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }
    vec4 albedoRead = texture(u_gAlbedoSmoothness, v_uv);
    vec4 normalRead = texture(u_gNormal, v_uv);
    vec3 normal = normalRead.xyz;
    vec3 color = albedoRead.rgb;
    float smoothness = albedoRead.w;
    vec3 fragWorldPos = ReconstructWorldPos(v_uv, depth);

    vec3 invLightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    vec3 lightColor = DIRECTIONAL_LIGHT.color * DIRECTIONAL_LIGHT.intensity;
    vec3 diffuseLight = max(dot(normal, invLightDir), 0.0) * color * lightColor;
    vec3 viewDir = normalize(VIEW_POS - fragWorldPos);
    vec3 halfway = normalize(viewDir + invLightDir);
    float specPower = pow(max(dot(normal, halfway), 0.0), exp2(25.0 * smoothness + 1.0));
    vec3 specularColor = specPower * color * lightColor;

    // Skybox reflections
    float lodLevel = (1.0 - smoothness) * 8.0;
    vec3 reflectionDir = reflect(-viewDir, normal);
    vec3 reflectionColor = textureLod(u_Skybox, reflectionDir, lodLevel).xyz;
    vec3 environmentSpecular = reflectionColor * color * smoothness;

    vec3 outcolor = diffuseLight + specularColor + environmentSpecular;
    FragColor = vec4(outcolor,  1.0);
}
















