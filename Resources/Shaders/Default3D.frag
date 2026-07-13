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
uniform vec4 _BASE_MAP_TO;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;
uniform float _SPECULAR_POWER;

uniform sampler2D _BASE_MAP;
uniform sampler2D _NORMAL_MAP;

void main(){
    vec2 uv = (out_uv * _BASE_MAP_TO.xy) + _BASE_MAP_TO.zw;
    vec4 texColor = texture(_BASE_MAP, uv);

    vec3 norm = normalize(out_normal);
    vec3 lightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    // Diffuse shading
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * DIRECTIONAL_LIGHT.color;
    // Specular shading
    vec3 viewDir = normalize(VIEW_POS - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 25);
    vec3 specular = _SPECULAR_POWER * spec * DIRECTIONAL_LIGHT.color;
    // Ambient + Diffuse + Specular
    vec3 lightingTotal = (_AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY) + diffuse + specular;

    FragColor = normalize(texColor * out_vertColor * vec4(lightingTotal, 1.0));
}