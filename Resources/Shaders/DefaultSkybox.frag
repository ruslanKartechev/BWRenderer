#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec3 FragNormal;
layout(location = 2) out vec2 FragReflection;

in vec3 TexCoords;
uniform samplerCube _SKYBOX;
uniform float _BRIGHTNESS;

void main(){
    FragColor = texture(_SKYBOX, TexCoords) * _BRIGHTNESS;
    FragReflection = vec2(0.0);

}