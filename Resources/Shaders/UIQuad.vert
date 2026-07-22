#version 450 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
out vec2 v_uv;

uniform vec4 u_PositionSize;
uniform vec2 u_ScreenResolution;

void main(){
    vec2 pixelPos = position.xy * u_PositionSize.zw + u_PositionSize.xy;
    vec2 ndcPos = (pixelPos / u_ScreenResolution) * 2.0 - 1.0;
    gl_Position = vec4(ndcPos.x, ndcPos.y, 0.0, 1.0);
    v_uv = uv;

}
