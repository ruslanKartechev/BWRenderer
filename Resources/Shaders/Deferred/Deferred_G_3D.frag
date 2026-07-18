#version 450 core

layout(location = 0) out vec4 gAlbedoSmoothness;
layout(location = 1) out vec4 gNormal;

layout(binding = 0) uniform sampler2D _BASE_MAP;
layout(binding = 1) uniform sampler2D _NORMAL_MAP;
//layout(binding = 2) uniform samplerCube _SKYBOX;


struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

in vec4 out_vertColor;
in vec2 v_uv;
in vec3 out_normal;
in mat3 out_TBN;

in vec3 FragPos;

uniform float _METALLIC;
uniform float _SMOOTHNESS;
uniform float _SSR_POWER;

uniform Light DIRECTIONAL_LIGHT;
uniform vec4 _BASE_MAP_TO;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;



void main(){
    vec2 uv = (v_uv * _BASE_MAP_TO.xy) + _BASE_MAP_TO.zw;
    vec4 texColor = texture(_BASE_MAP, uv) * out_vertColor;
    vec3 normal = texture(_NORMAL_MAP, uv).xyz;
    normal = normal * 2.0 - 1.0;
    normal = normalize(out_TBN * normal);

    gAlbedoSmoothness = vec4(texColor.rgb, _SMOOTHNESS);
    gNormal = vec4(normal, _SSR_POWER);
}