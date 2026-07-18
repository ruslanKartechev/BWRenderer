#version 450 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 FragNormal;
layout(location = 2) out vec2 FragReflection;


layout(binding = 0) uniform sampler2D _BASE_MAP;
layout(binding = 1) uniform sampler2D _NORMAL_MAP;
layout(binding = 2) uniform samplerCube _SKYBOX;


struct Light{
    vec3 direction;
    vec3 color;
    float intensity;
};

in vec4 out_vertColor;
in vec2 v_uv;
in vec3 out_normal;
in mat3 out_TBN;

in vec3 FragPos;

uniform float _METALLIC;
uniform float _SMOOTHNESS;
uniform float _SSR_POWER;

uniform Light DIRECTIONAL_LIGHT;
uniform vec4 _BASE_MAP_TO;
uniform vec3 VIEW_POS;
uniform vec3 _AMBIENT_LIGHT_COLOR;
uniform float _AMBIENT_LIGHT_INTENSITY;



void main(){
    vec2 uv = (v_uv * _BASE_MAP_TO.xy) + _BASE_MAP_TO.zw;
    vec4 texColor = texture(_BASE_MAP, uv) * out_vertColor;
//    vec3 normal = normalize(out_normal);
    vec3 normal =  texture(_NORMAL_MAP, uv).xyz;
    normal = normal * 2.0 - 1.0; // convert to [-1, 1]
    normal = normalize (out_TBN * normal);

    vec3 invLightDir = normalize(-DIRECTIONAL_LIGHT.direction);
    vec3 lightColor = DIRECTIONAL_LIGHT.color.xyz * DIRECTIONAL_LIGHT.intensity;
    // Diffuse color
    vec3 diffCol = texColor.xyz * (1.0 - _METALLIC);
    vec3 diffuseLight = max(dot(normal, invLightDir), 0.0) * diffCol * lightColor;
    // Specular color
    vec3 f0 = mix(vec3(0.04), texColor.xyz, _METALLIC);
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
    vec3 environmentSpecular = clamp(reflectionColor * f0 * _SMOOTHNESS, vec3(0.0), vec3(1.0));
    // Ambient
    vec3 ambient = _AMBIENT_LIGHT_COLOR * _AMBIENT_LIGHT_INTENSITY;
    vec3 finalColor = ambient + environmentSpecular + specularColor + diffuseLight;
    FragColor = vec4(finalColor, 1.0);

    FragNormal = vec4(normal, _SSR_POWER);
    FragReflection = vec2(_SMOOTHNESS, _METALLIC);

}