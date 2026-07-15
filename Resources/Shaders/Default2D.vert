#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 uv;

out vec4 out_vertColor;
out vec2 v_uv;

//uniform float _Time;


void main(){
    gl_Position = vec4(position, 1.0, 1.0);
    v_uv = uv;
}
