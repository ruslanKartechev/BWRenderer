#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
out vec4 out_vertColor;
out vec2 out_uv;

uniform mat4 MATRIX_MVP;
uniform float GLOBAL_TIME;


void main(){
    gl_Position = MATRIX_MVP * vec4(position, 1.0);
    out_uv = uv;
    out_vertColor = vec4(0.5, 1.0, 0.5, 1.0);
}
