#version 450 core

layout (location = 0) in vec2 position;

out vec4 out_vertColor;
out vec2 v_uv;
out vec3 out_normal;
//out mat3 out_TBN;
out vec3 FragPos;

layout (binding = 0) uniform sampler2D _HEIGHT_MAP;

uniform mat4 MATRIX_MODEL;
uniform mat4 MATRIX_VIEW;
uniform mat4 MATRIX_PROJECTION;

uniform float GLOBAL_TIME;
uniform vec4 _COLOR_TINT;
uniform vec4 _SIZE;

uniform vec2 _CAMERA_POSITION;
uniform float _LOD_SCALE;
uniform float _IS_SKIRT;

const float HeightPower = 3.0;

float GetWorldPosHeight(vec2 worldPos){
    vec2 uv = (worldPos.xy + _SIZE.xy * 0.5) / _SIZE.xy;
    return texture(_HEIGHT_MAP, fract(uv)).r;
}

float SampleCoarseHeight(vec2 worldPos){
    float cStep = _LOD_SCALE * 2.0; // next one is twice as big
    vec2 origin = floor(worldPos / cStep) * cStep;
    vec2 t = (worldPos - origin) / cStep;

    float h00 = GetWorldPosHeight(origin);
    float h10 = GetWorldPosHeight(origin + vec2(cStep, 0.0));
    float h01 = GetWorldPosHeight(origin + vec2(0.0, cStep));
    float h11 = GetWorldPosHeight(origin + vec2(cStep, cStep));
    // Barycentric interpolation
    if (t.x + t.y < 1.0) {
        // Upper Left Triangle
        return h00 + t.x * (h10 - h00) + t.y * (h01 - h00);
    } else {
        // Bottom Right Triangle
        return h11 + (1.0 - t.x) * (h01 - h11) + (1.0 - t.y) * (h10 - h11);
    }
}

float CalculateHeight(float hRaw, float offset){
    float h = pow(hRaw * _SIZE.z, HeightPower) - offset;
    return h;
}



void main(){
    vec2 halfSize = _SIZE.xy * 0.5;
    float hScale = _SIZE.z;
    float downOffset = pow(0.5 * hScale, HeightPower);

    vec3 flatPos = (MATRIX_MODEL * vec4(position.x, 0.0, position.y, 1.0)).xyz;
    v_uv = (flatPos.xz + halfSize) / _SIZE.xy;
    // v_uv = fract(v_uv);
    // Calculate Alpha Morph factor based on continuous camera distance
    vec2 d = abs(flatPos.xz - _CAMERA_POSITION) / _LOD_SCALE;
    float dMax = max(d.x, d.y);
    float alpha = clamp((dMax - 24.0) / 6.0, 0.0, 1.0);
    float hRaw = texture(_HEIGHT_MAP, v_uv).r;
    float hCoarsed = SampleCoarseHeight(flatPos.xz);
    float hBlendedRaw = mix(hRaw, hCoarsed, alpha);
    float h = CalculateHeight(hRaw, downOffset);

    if (_IS_SKIRT > 0.5) {
        float isBottom = mod(float(gl_VertexID), 2.0);
        float hCoarseFinal = CalculateHeight(hCoarsed, downOffset);
        float epsilonDrop = 0.01 * _LOD_SCALE;
        h = mix(h, hCoarseFinal, isBottom);
    }

    vec4 localPos = vec4(position.x, h, position.y, 1.0);
    FragPos = (MATRIX_MODEL * localPos).xyz;
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * localPos;

    // Step by _LOD_SCALE
    vec2 right = flatPos.xz + vec2(_LOD_SCALE, 0.0);
    vec2 frw = flatPos.xz + vec2(0.0, _LOD_SCALE);
    float hRightRaw = mix(GetWorldPosHeight(right), SampleCoarseHeight(right), alpha);
    float hFrwRaw = mix(GetWorldPosHeight(frw), SampleCoarseHeight(frw), alpha);
//    float hRightRaw = GetWorldPosHeight(right);
//    float hFrwRaw = GetWorldPosHeight(frw);

    float hRight = CalculateHeight(hRightRaw, downOffset);
    float hUp    = CalculateHeight(hFrwRaw, downOffset);
    vec3 worldTangent   = normalize(vec3(_LOD_SCALE, hRight - h, 0.0));
    vec3 worldBitangent = normalize(vec3(0.0, hUp - h, _LOD_SCALE));
    vec3 worldNormal    = normalize(cross(worldBitangent, worldTangent));
    out_normal = worldNormal;
    out_vertColor = vec4(_COLOR_TINT.xyz, 1.0);
}