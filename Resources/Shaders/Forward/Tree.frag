#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;
layout(location = 2) out vec2 FragReflection;

struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

in vec4 out_vertColor;
in vec2 v_uv;
in vec3 out_normal;
in vec3 FragPos;

layout(binding = 0) uniform sampler2D _BASE_MAP;

uniform Light DIRECTIONAL_LIGHT;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;
uniform float _SPECULAR_POWER;
uniform float _ALPHA_CLIP_VALUE;


void main(){
    vec4 texColor = texture(_BASE_MAP, v_uv);
    if(texColor.a < _ALPHA_CLIP_VALUE){
        discard;
        return;
    }

    vec3 norm = normalize(out_normal);
    vec3 invLightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    // Diffuse shading
    vec3 diffuse = max(dot(norm, invLightDir), 0.0) * DIRECTIONAL_LIGHT.color;
    // Specular shading
    vec3 viewDir = normalize(VIEW_POS - FragPos);
    vec3 reflectDir = reflect(-invLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
    // Ambient
    vec3 ambient = (_AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY);

    vec3 lightingTotal = ambient + diffuse + spec;
    FragColor = normalize(texColor * out_vertColor * vec4(lightingTotal, 1.0));

    FragNormal = vec4(norm, 0.0);
    FragReflection = vec2(0.0, 0.0);
}