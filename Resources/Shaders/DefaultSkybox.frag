#version 330 core

out vec4 FragColor;

in vec3 TexCoords;
uniform samplerCube _SKYBOX;
uniform float _BRIGHTNESS;

void main(){
    FragColor = texture(_SKYBOX, TexCoords) * _BRIGHTNESS;

}