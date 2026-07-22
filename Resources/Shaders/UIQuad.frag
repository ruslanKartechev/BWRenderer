#version 450 core
out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D mainTex;
uniform float _GAMMA = 1.0;

void main(){
    vec4 color = texture(mainTex, v_uv);
//    color = pow(color, vec3(1.0 / _GAMMA));
    float c = color.r;
//    c = (c + 1.0) / 2.0;
    FragColor = vec4(vec3(c), 1.0);
}