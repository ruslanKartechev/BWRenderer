#version 450 core
#define DEBUG_COLORS__

layout(location = 0) out vec4 gAlbedoSmoothness;
layout(location = 1) out vec4 gNormal;

layout(binding = 1) uniform sampler2D _SPLAT_MAP;
layout(binding = 2) uniform sampler2D _TERRAIN_TEX_1;
layout(binding = 3) uniform sampler2D _TERRAIN_TEX_2;
layout(binding = 4) uniform sampler2D _TERRAIN_TEX_3;
layout(binding = 5) uniform sampler2D _TERRAIN_TEX_4;

in vec4 out_vertColor;
in vec2 v_uv;
in vec3 out_normal;
//in mat3 out_TBN;
in vec3 FragPos;

uniform float _METALLIC;
uniform float _SMOOTHNESS;
uniform float _SSR_POWER;
uniform vec4 _BASE_MAP_TO;
uniform vec4 _SIZE;


void main(){
    vec2 uv = v_uv;
    vec2 scaledUV = v_uv * _SIZE.xy;
    scaledUV = fract(scaledUV);

    vec4 splatMask = texture(_SPLAT_MAP, uv);
    float w1 = splatMask.x;
    float w2 = splatMask.y;
    float w3 = splatMask.z;
    float w4 = splatMask.w;

    vec4 col1 = texture(_TERRAIN_TEX_1, scaledUV) * w1;
    vec4 col2 = texture(_TERRAIN_TEX_2, scaledUV) * w2;
    vec4 col3 = texture(_TERRAIN_TEX_3, scaledUV) * w3;
    vec4 col4 = texture(_TERRAIN_TEX_4, scaledUV) * 0;
    vec4 finalColor = col1 + col2 + col3 + col4;
    finalColor = clamp(finalColor, vec4(0.0), vec4(1.0));

//    finalColor = vec4(v_uv.x, v_uv.y, 0.0, 1.0);

    gAlbedoSmoothness = vec4(finalColor.xyz, _SMOOTHNESS);
    gNormal = vec4(out_normal, _SSR_POWER);

    gAlbedoSmoothness = vec4(uv, 0.0, _SMOOTHNESS);
#ifdef DEBUG_COLORS
    gAlbedoSmoothness = vec4(out_vertColor.rgb, _SMOOTHNESS);
#endif
}