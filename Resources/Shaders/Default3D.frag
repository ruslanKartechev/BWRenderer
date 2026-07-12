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
uniform sampler2D _BASE_MAP;

void main(){
    float specularStrength = 0.5;
    vec3 norm = normalize(out_normal);
    vec3 lightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    // Diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * DIRECTIONAL_LIGHT.color;
    // Specular shading
    vec3 viewDir = normalize(VIEW_POS - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
    vec3 specular = specularStrength * spec * DIRECTIONAL_LIGHT.color;
    // Ambient + Diffuse + Specular
    vec3 result = (_AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY) + diffuse + specular;

    float t = 0.1;
    FragColor = (texture(_BASE_MAP, out_uv) + vec4(t,t,t,t)) * out_vertColor * vec4(result, 1.0);
}