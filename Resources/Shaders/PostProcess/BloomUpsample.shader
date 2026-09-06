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
uniform float u_FilterRadius = 0.005;

// Upsampling weights
const float W0 = 0.25;
const float W1 = 0.125;
const float W2 = 0.0625;

vec3 Sample(){
    float x = u_FilterRadius;
    float y = u_FilterRadius;

    // sampling 13 neighbours
    vec3 center = texture(u_InputTexture, vec2(v_uv.x,  v_uv.y)).rgb;
    vec3 a = texture(u_InputTexture, vec2(v_uv.x - x,   v_uv.y + y)).rgb;
    vec3 b = texture(u_InputTexture, vec2(v_uv.x,       v_uv.y + y)).rgb;
    vec3 c = texture(u_InputTexture, vec2(v_uv.x + x,   v_uv.y + y)).rgb;
    vec3 d = texture(u_InputTexture, vec2(v_uv.x - x,   v_uv.y)).rgb;
    vec3 f = texture(u_InputTexture, vec2(v_uv.x + x,   v_uv.y)).rgb;
    vec3 g = texture(u_InputTexture, vec2(v_uv.x - x,   v_uv.y - y)).rgb;
    vec3 h = texture(u_InputTexture, vec2(v_uv.x,       v_uv.y - y)).rgb;
    vec3 i = texture(u_InputTexture, vec2(v_uv.x + x,   v_uv.y - y)).rgb;

    vec3 color = center * W0;
    color += (b + d + f + h) * W1;
    color += (a + c + g + i) * W2;
    return color;
}

void main() {
    vec3 color = Sample() + vec3(0.0, 0.0, 0.0);
    FragColor = vec4(color, 1.0);

//    FragColor = vec4(u_FilterRadius,u_FilterRadius,u_FilterRadius, 1.0);

}
#endsection
