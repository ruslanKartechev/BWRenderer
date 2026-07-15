#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec2 FragReflection;
in vec4 vertColor;

void main(){
    FragColor = vertColor;
    FragNormal = vec3(0.0);
    FragReflection = vec2(0.0);
}