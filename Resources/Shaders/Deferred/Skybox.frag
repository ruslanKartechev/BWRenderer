#version 450 core

layout(location = 0) out vec4 FragColor;

layout(binding = 0) uniform samplerCube _SKYBOX;
uniform float _BRIGHTNESS;

in vec3 TexCoords;


void main(){
    FragColor = texture(_SKYBOX, TexCoords) * _BRIGHTNESS;

}