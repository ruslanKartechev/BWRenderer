#version 450 core
out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_InputTexture;
uniform vec2 u_TexelSize;

// Standard Gaussian weights
const float W0 = 0.125;
const float W1 = 0.03125;
const float W2 = 0.0625;
const float W3 = 0.125;

vec3 Sample(){
    float x = u_TexelSize.x;
    float y = u_TexelSize.y;
    // sampling 13 neighbours
    vec3 center = texture(u_InputTexture, vec2(v_uv.x, v_uv.y)).rgb;

    vec3 a = texture(u_InputTexture, vec2(v_uv.x - 2.0 * x,  v_uv.y + 2.0 * y)).rgb;
    vec3 b = texture(u_InputTexture, vec2(v_uv.x,            v_uv.y + 2.0 * y)).rgb;
    vec3 c = texture(u_InputTexture, vec2(v_uv.x + 2.0 * x,  v_uv.y + 2.0 * y)).rgb;
    vec3 d = texture(u_InputTexture, vec2(v_uv.x - 2.0 * x,  v_uv.y)).rgb;
    vec3 f = texture(u_InputTexture, vec2(v_uv.x + 2.0 * x,  v_uv.y)).rgb;
    vec3 g = texture(u_InputTexture, vec2(v_uv.x - 2.0 * x,  v_uv.y - 2.0 * y)).rgb;
    vec3 h = texture(u_InputTexture, vec2(v_uv.x,            v_uv.y - 2.0 * y)).rgb;
    vec3 i = texture(u_InputTexture, vec2(v_uv.x + 2.0 * x,  v_uv.y - 2.0 * y)).rgb;
    vec3 j = texture(u_InputTexture, vec2(v_uv.x - x,        v_uv.y + y)).rgb;
    vec3 k = texture(u_InputTexture, vec2(v_uv.x + x,        v_uv.y + y)).rgb;
    vec3 l = texture(u_InputTexture, vec2(v_uv.x - x,        v_uv.y - y)).rgb;
    vec3 m = texture(u_InputTexture, vec2(v_uv.x + x,        v_uv.y - y)).rgb;

    vec3 color = center * W0;
    color += (a + c + g + i) * W1;
    color += (b + d + f + h) * W2;
    color += (j + k + l + m) * W3;
    return color;
}

void main() {
    vec3 color = Sample();
    FragColor = vec4(color, 1.0);
}