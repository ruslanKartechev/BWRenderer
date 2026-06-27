#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;

out vec4 out_vertColor;
out vec2 out_uv;
out vec3 out_normal;
out vec3 FragPos;

uniform mat4 MATRIX_MVP;
uniform float GLOBAL_TIME;

void main(){
    gl_Position = MATRIX_MVP * vec4(position, 1.0);
    FragPos = gl_Position.xyz;
//    out_normal = vec3(0.0, 1.0, 0.0);
    out_normal = normalize(normal);
    out_uv = uv;
    out_vertColor = vec4(0.2, 0.1, 0.2, 1.0);
}
