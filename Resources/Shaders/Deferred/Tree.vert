#version 450 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in mat4 modelMatrix;

out vec4 out_vertColor;
out vec2 v_uv;
out vec3 out_normal;
out vec3 FragPos;

//uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;
uniform vec4 _COLOR_TINT;
uniform sampler2D _BASE_MAP;

void main(){
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * modelMatrix) * vec4(position, 1.0);
    FragPos = (modelMatrix * vec4(position, 1.0)).xyz;
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    out_normal = normalize(normalMatrix * normal);
    v_uv = uv;
    out_vertColor = vec4(_COLOR_TINT.xyz, 1);

}