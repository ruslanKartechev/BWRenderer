#version 450 core

layout(location = 0) out vec4 u_gAlbedoSmoothness;
layout(location = 1) out vec4 u_gNormal;

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
    u_gAlbedoSmoothness = vec4(normalize(texColor * out_vertColor).xyz, 0.0);
    u_gNormal = vec4(normalize(out_normal), 0.0); // 0.0 for SSR power
}