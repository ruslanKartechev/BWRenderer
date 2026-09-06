#version 450 core


#section Vertex
layout (location = 0) in vec3 position;
out vec3 TexCoords;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;

void main(){
    TexCoords = position;
    vec4 pos = MATRIX_PROJECTION * MATRIX_VIEW * vec4(position, 1.0);
    gl_Position = pos.xyww;

}
#endsection


#section Fragment

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform samplerCube _SKYBOX;
uniform float _BRIGHTNESS;
in vec3 TexCoords;

void main(){
    FragColor = texture(_SKYBOX, TexCoords) * _BRIGHTNESS;
}

#endsection