#version 330 core

layout (location = 0) out vec4 FragColor;

uniform sampler2D mainTex;
in vec4 out_vertColor;
in vec2 v_uv;


void main(){
    vec4 texColor = texture(mainTex, v_uv);
//    FragColor = vec4(1.0, 0.0,0.0, 1.0);
    FragColor = texColor;
}