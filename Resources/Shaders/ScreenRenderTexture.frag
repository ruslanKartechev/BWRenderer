#version 330 core
out vec4 FragColor;

in vec2 out_uv;

uniform sampler2D screenTexture;

void main(){
    FragColor = texture(screenTexture, out_uv);
//    FragColor = vec4(out_uv.x, out_uv.y, 1.0, 1.0);
}