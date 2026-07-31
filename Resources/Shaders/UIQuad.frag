#version 450 core
out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D mainTex;
uniform float _GAMMA = 1.0;

void main(){
    vec4 color = texture(mainTex, v_uv);
    color = pow(color, vec4(1.0 / _GAMMA));
    FragColor = color;
}