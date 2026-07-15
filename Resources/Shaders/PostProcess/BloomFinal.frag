#version 330 core
out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_MainTex;
uniform sampler2D u_BloomTex;
uniform float u_Exposure = 1.0;

void main() {
    vec3 baseColor = texture(u_MainTex, v_uv).rgb;
    vec3 bloomColor = texture(u_BloomTex, v_uv).rgb;
    baseColor += bloomColor;
    float x = u_Exposure;
    vec3 result = vec3(1.0) - exp(-baseColor * x);

    FragColor = vec4(result, 1.0);
//    FragColor = vec4(bloomColor, 1.0);


}