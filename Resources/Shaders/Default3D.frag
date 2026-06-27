#version 330 core

out vec4 FragColor;

in vec4 out_vertColor;
in vec2 out_uv;

void main(){
    FragColor = out_vertColor;
}