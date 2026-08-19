#version 450 core
out vec4 FragColor;

in vec2 v_uv;

layout (binding = 0) uniform sampler2D mainTex;
uniform float _GAMMA = 1.0;

void main(){
    vec4 color = texture(mainTex, fract(v_uv), 0.0);
//    color = pow(color, vec4(1.0 / _GAMMA));
    float r = color.r / 1.0f;
    r = r * r * r;
    r = r * 3.0;
    FragColor = vec4(r, r, r, 1.0);
}