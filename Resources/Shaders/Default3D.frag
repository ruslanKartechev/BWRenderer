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
in mat3 out_TBN;
in vec3 FragPos;

uniform float _METALLIC;
uniform float _SMOOTHNESS;

uniform Light DIRECTIONAL_LIGHT;
uniform vec4 _BASE_MAP_TO;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;

uniform sampler2D _BASE_MAP;
uniform sampler2D _NORMAL_MAP;
uniform samplerCube _SKYBOX;


void main(){
    vec2 uv = (out_uv * _BASE_MAP_TO.xy) + _BASE_MAP_TO.zw;
    vec4 texColor = texture(_BASE_MAP, uv) * out_vertColor;
//    vec3 normal = normalize(out_normal);
    vec3 normal =  texture(_NORMAL_MAP, uv).xyz;
    normal = normal * 2.0 - 1.0; // convert to [-1, 1]
    normal = normalize (out_TBN * normal);

    vec3 invLightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    vec3 lightColor = DIRECTIONAL_LIGHT.color.xyz;
    // Diffuse color
    vec3 diffCol = texColor.xyz * (1.0 - _METALLIC);
    vec3 diffuseLight = max(dot(normal, invLightDir), 0.0) * diffCol * lightColor;
    // Specular color
    vec3 f0 =  mix(vec3(0.04), texColor.xyz, _METALLIC);
    vec3 viewDir = normalize(VIEW_POS - FragPos);
    vec3 halfway = normalize(viewDir + invLightDir);
    float specP = max(dot(normal, halfway), 0.0); // Blinn model
    float shiny = exp2(25.0 * _SMOOTHNESS + 1.0); // Specular power
    float specPower = pow(specP, shiny);
    vec3 specularColor = specPower * f0 * lightColor;
    // Reflections
    vec3 reflectionDir = reflect(-viewDir, normal);
    float lodLevel = (1.0 - _SMOOTHNESS) * 8.0;
    vec3 reflectionColor = textureLod(_SKYBOX, reflectionDir, lodLevel).xyz;
    vec3 environmentSpecular = reflectionColor * f0 * _SMOOTHNESS;
    // Ambient
    vec3 ambient = _AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY;
    vec3 finalColor = ambient + environmentSpecular + specularColor + diffuseLight;
    FragColor = vec4(finalColor, 1.0);

}