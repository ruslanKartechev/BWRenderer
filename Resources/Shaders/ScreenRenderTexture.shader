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
layout(binding = 0) uniform sampler2D mainTex;
uniform float _GAMMA = 1.0;

void main(){
    vec3 color = texture(mainTex, v_uv).rgb;
    color = pow(color, vec3(1.0 / _GAMMA));
    FragColor = vec4(color, 1.0);
}
#endsection
