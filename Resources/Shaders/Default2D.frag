#version 330 core

out vec4 FragColor;

in vec4 out_vertColor;
in vec2 out_uv;

uniform sampler2D  mainTex;

void main(){
    vec4 texColor = texture(mainTex, out_uv);

    FragColor = out_vertColor * texColor;
}