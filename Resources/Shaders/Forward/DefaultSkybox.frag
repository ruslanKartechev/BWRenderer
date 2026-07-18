#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec2 FragReflection;

layout(binding = 0) uniform samplerCube _SKYBOX;
uniform float _BRIGHTNESS;

in vec3 TexCoords;


void main(){
    FragColor = texture(_SKYBOX, TexCoords) * _BRIGHTNESS;
    FragNormal = vec3(0.0);
    FragReflection = vec2(0.0);

}