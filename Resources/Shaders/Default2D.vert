#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
out vec4 out_vertColor;
out vec2 out_uv;

//uniform float _Time;


void main(){
    gl_Position = vec4(position, 1.0, 1.0);
    out_uv = uv;
    out_vertColor = color;
}
