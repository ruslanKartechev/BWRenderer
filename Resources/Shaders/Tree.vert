#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec4 out_vertColor;
out vec2 out_uv;
out vec3 out_normal;
out vec3 FragPos;

uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;
uniform vec4 _COLOR_TINT;
uniform sampler2D _BASE_MAP;

void main(){
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * vec4(position, 1.0);
    FragPos = (MATRIX_MODEL * vec4(position, 1.0)).xyz;
    mat3 normalMatrix = mat3(transpose(inverse(MATRIX_MODEL)));
    out_normal = normalize(normalMatrix * normal);
    out_uv = uv;
    out_vertColor = vec4(_COLOR_TINT.xyz, 1);

}