#version 330 core
out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_MainTex;
uniform float u_Threshold = 1.0; // Anything brighter than 1.0 gets bloomed

void main() {
    vec3 color = texture(u_MainTex, v_uv).rgb;
    // Calculate brightness (dot product with luminance weights)
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > u_Threshold) {
        FragColor = vec4(color, 1.0);
    } else {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}