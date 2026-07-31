#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec4 out_vertColor;
out vec2 v_uv;
out vec3 out_normal;
out mat3 out_TBN;
out vec3 FragPos;

layout (binding = 0) uniform sampler2D _HEIGHT_MAP;

uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;

uniform float GLOBAL_TIME;
uniform vec4 _COLOR_TINT;
uniform float _HEIGHT_SCALE = 10.0;
uniform vec4 _SIZE;

const float HPower = 3.0;
const float gridStep = 1.0;

void main(){
    v_uv = uv;
    vec2 globalUV = (uv / _SIZE.xy) + 0.5;

    float hRaw = texture(_HEIGHT_MAP, v_uv).r;
    float h = pow(hRaw * _HEIGHT_SCALE, HPower);
    vec3 pos = position + vec3(0.0, h, 0.0);
    FragPos = (MATRIX_MODEL * vec4(pos, 1.0)).xyz;
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * vec4(pos, 1.0);


    vec2 texel = 1.0 / textureSize(_HEIGHT_MAP, 0);
    float hRightRaw = texture(_HEIGHT_MAP, uv + vec2(texel.x, 0.0)).r;
    float hUpRaw    = texture(_HEIGHT_MAP, uv + vec2(0.0, texel.y)).r;

    float hRight = pow(hRightRaw * _HEIGHT_SCALE, HPower);
    float hUp    = pow(hUpRaw * _HEIGHT_SCALE, HPower);

    vec3 p      = vec3(0.0, h, 0.0);
    vec3 pRight = vec3(gridStep, hRight, 0.0);
    vec3 pUp    = vec3(0.0, hUp, gridStep);

    vec3 tangent = normalize(pRight - p);
    vec3 bitangent = normalize(pUp - p);
    vec3 generatedNormal = normalize(cross(bitangent, tangent));

    mat3 normalMatrix = mat3(transpose(inverse(MATRIX_MODEL)));
    out_normal = normalize(normalMatrix * generatedNormal);

    // TBN
    vec3 T = normalize(vec3(MATRIX_MODEL * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(MATRIX_MODEL * vec4(generatedNormal, 0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    out_TBN = mat3(T, B, N);

    out_vertColor = vec4(_COLOR_TINT.xyz, 1.0);
}