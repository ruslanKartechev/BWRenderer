#version 330 core

layout(location = 0) out vec4 FragColor;

in vec4 vertColor;

void main(){
    FragColor = vertColor;
}