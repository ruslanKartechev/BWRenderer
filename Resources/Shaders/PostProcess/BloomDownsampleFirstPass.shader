#version 450 core

#section Vertex
layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv;
out vec2 v_uv;

void main(){
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
    v_uv = uv;
}
#endsection


#section Fragment
out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_InputTexture;
uniform vec2 u_TexelSize;
uniform float u_Threshold = 0.9;

// Standard Gaussian weights
const float W0 = 0.125;
const float W1 = 0.03125;
const float W2 = 0.0625;
const float W3 = 0.125;

const vec3 colorKernel = vec3(0.9126, 0.8152, 0.9722); // vec3(0.2126, 0.7152, 0.0722)

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
    float brightness = dot(color, colorKernel);
    if(brightness < u_Threshold){
        color = vec3(0.0);
    }
    FragColor = vec4(color, 1.0);
}
#endsection
