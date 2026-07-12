#version 330 core

out vec4 FragColor;

in vec3 TexCoords;
uniform samplerCube _SKYBOX;

void main(){
//    FragColor = vec4(1.0,.5,1.0,1.0);
    FragColor = texture(_SKYBOX, TexCoords);
    FragColor = vec4(TexCoords, 1);
}