#version 330 core

out vec4 FragColor;

struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

in vec4 out_vertColor;
in vec2 out_uv;
in vec3 out_normal;
in vec3 FragPos;

uniform Light DIRECTIONAL_LIGHT;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;
uniform float _SPECULAR_POWER;
uniform float _ALPHA_CLIP_VALUE;
uniform sampler2D _BASE_MAP;


void main(){
    vec4 texColor = texture(_BASE_MAP, out_uv);
    if(texColor.a < _ALPHA_CLIP_VALUE){
        discard;
        return;
    }

    vec3 norm = normalize(out_normal);
    vec3 invLightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    // Diffuse shading
    vec3 diffuse = max(dot(norm, invLightDir), 0.0) * DIRECTIONAL_LIGHT.color;
    // Specular shading
    vec3 viewDir = normalize(VIEW_POS - FragPos);
    vec3 reflectDir = reflect(-invLightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
    // Ambient
    vec3 ambient = (_AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY);

    vec3 lightingTotal = ambient + diffuse + spec;
    FragColor = normalize(texColor * out_vertColor * vec4(lightingTotal, 1.0));
}