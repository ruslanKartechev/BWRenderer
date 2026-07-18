#version 450 core
out vec4 FragColor;
in vec2 v_uv;

layout (binding = 0) uniform sampler2D u_MainTex;
layout (binding = 1) uniform sampler2D u_BlendWith;
uniform float u_Exposure = 1.0;

void main() {
    vec3 baseColor = texture(u_MainTex, v_uv).rgb;
    vec3 bloomColor = texture(u_BlendWith, v_uv).rgb;
    // FragColor = vec4(bloomColor * 1.0 + baseColor * 0.1, 1.0);
    // return;
    vec3 result = vec3(1.0) - exp(-bloomColor * u_Exposure);
    baseColor += clamp(result, vec3(0.0), vec3(1.0));
    baseColor = clamp(baseColor, vec3(0.0), vec3(1.0));
    FragColor = vec4(baseColor, 1.0);

}