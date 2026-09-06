#version 450 core

#section Vertex
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
#endsection


#section Fragment
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
#endsection