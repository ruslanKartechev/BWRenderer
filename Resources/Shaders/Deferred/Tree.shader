#version 450 core

#section Vertex

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 normal;
layout (location = 3) in mat4 modelMatrix;

out vec4 out_vertColor;
out vec2 v_uv;
out vec3 out_normal;
out vec3 FragPos;

//uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;
uniform vec4 _COLOR_TINT;
uniform sampler2D _BASE_MAP;

void main(){
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * modelMatrix) * vec4(position, 1.0);
    FragPos = (modelMatrix * vec4(position, 1.0)).xyz;
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    out_normal = normalize(normalMatrix * normal);
    v_uv = uv;
    out_vertColor = vec4(_COLOR_TINT.xyz, 1);

}
#endsection



#section Fragment

layout(location = 0) out vec4 u_gAlbedoSmoothness;
layout(location = 1) out vec4 u_gNormal;

struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

in vec4 out_vertColor;
in vec2 v_uv;
in vec3 out_normal;
in vec3 FragPos;

layout(binding = 0) uniform sampler2D _BASE_MAP;

uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;
uniform float _SPECULAR_POWER;
uniform float _ALPHA_CLIP_VALUE;


void main(){
    vec4 texColor = texture(_BASE_MAP, v_uv);
    if(texColor.a < _ALPHA_CLIP_VALUE){
        discard;
        return;
    }
    u_gAlbedoSmoothness = vec4(normalize(texColor * out_vertColor).xyz, 0.0);
    u_gNormal = vec4(normalize(out_normal), 0.0); // 0.0 for SSR power
}
#endsection
