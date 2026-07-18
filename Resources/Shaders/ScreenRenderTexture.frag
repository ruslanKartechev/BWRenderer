#version 450 core
out vec4 FragColor;

in vec2 v_uv;

layout(binding = 0) uniform sampler2D screenTexture;
uniform float _GAMMA = 1.0;

void main(){
    vec3 color = texture(screenTexture, v_uv).rgb;
    color = pow(color, vec3(1.0 / _GAMMA));
    FragColor = vec4(color, 1.0);
}