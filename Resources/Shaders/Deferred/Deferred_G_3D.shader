#version 450 core

#section Vertex


layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

out vec4 out_vertColor;
out vec2 v_uv;
out vec3 out_normal;
out mat3 out_TBN;
out vec3 FragPos;

uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;

uniform float GLOBAL_TIME;
uniform vec4 _COLOR_TINT;
uniform sampler2D _BASE_MAP;

void main(){
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * vec4(position, 1.0);
    FragPos = (MATRIX_MODEL * vec4(position, 1.0)).xyz;
    mat3 normalMatrix = mat3(transpose(inverse(MATRIX_MODEL)));
    out_normal = normalize(normalMatrix * normal);
    v_uv = uv;
    out_vertColor = vec4(_COLOR_TINT.xyz, 1);

    vec3 T = normalize(vec3(MATRIX_MODEL * vec4(tangent, 0.0)));
    vec3 N = normalize(vec3(MATRIX_MODEL * vec4(normal, 0.0)));
    T = normalize(T - dot(T, N) * N); // correction of direction so that they are perpendicular
    vec3 B = cross(N, T);
    out_TBN = mat3(T, B, N);
}
#endsection




#section Fragment
layout(location = 0) out vec4 gAlbedoSmoothness;
layout(location = 1) out vec4 gNormal;
layout(binding = 0) uniform sampler2D _BASE_MAP;
layout(binding = 1) uniform sampler2D _NORMAL_MAP;

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
uniform vec4 _BASE_MAP_TO;

void main(){
    vec2 uv = (v_uv * _BASE_MAP_TO.xy) + _BASE_MAP_TO.zw;
    vec4 texColor = texture(_BASE_MAP, uv) * out_vertColor;
    vec3 normal = texture(_NORMAL_MAP, uv).xyz;
    normal = normal * 2.0 - 1.0;
    normal = normalize(out_TBN * normal);
    gAlbedoSmoothness = vec4(texColor.rgb, _SMOOTHNESS);
    //    gAlbedoSmoothness = vec4(1.0, 1.0, 1.0, 1.0);
    gNormal = vec4(normal, _SSR_POWER);
}
#endsection
