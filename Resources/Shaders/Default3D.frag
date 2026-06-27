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
//
uniform Light GLOBAL_LIGHT;
uniform vec3 AMBIENT_LIGHT_COLOR;
uniform vec4 VIEW_POS;
uniform float AMBIENT_LIGHT_INTENSITY;

void main(){
    float specularStrength = 0.5;
    vec3 norm = normalize(out_normal);
    vec3 lightDir = normalize(-GLOBAL_LIGHT.direction).xyz;

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * GLOBAL_LIGHT.color;

    vec3 viewDir = normalize(VIEW_POS.xyz - FragPos);
    vec3 reflectDir = reflect(-(GLOBAL_LIGHT.direction).xyz, out_normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * GLOBAL_LIGHT.color;

    vec3 result = (AMBIENT_LIGHT_COLOR * AMBIENT_LIGHT_INTENSITY + diffuse + specular); //* out_vertColor.xyz;

    FragColor = vec4(result, 1.0);
}