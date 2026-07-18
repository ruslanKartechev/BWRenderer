#version 450 core

layout(location = 0) out vec4 FragColor;
in vec2 v_uv;

uniform sampler2D u_DepthTex;

const float NEAR_PLANE = 0.1;
const float FAR_PLANE = 500.0;



float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}


void main(){
    float depth = texture(u_DepthTex, v_uv).r;
    depth = LinearizeDepth(depth) / FAR_PLANE;
    depth = clamp(depth, 0.0, 1.0);
    FragColor = vec4(depth, depth, depth, 1.0);
}
