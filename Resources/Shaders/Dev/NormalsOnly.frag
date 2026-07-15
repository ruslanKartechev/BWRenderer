#version 330 core

layout(location = 0) out vec4 FragColor;
in vec2 v_uv;

uniform float NEAR_PLANE;
uniform float FAR_PLANE;

uniform sampler2D u_NormalTex;



void main(){
    vec3 normals = texture(u_NormalTex, v_uv).rgb;
    normals = normalize(normals);
    FragColor = vec4(normals, 1.0);
}
