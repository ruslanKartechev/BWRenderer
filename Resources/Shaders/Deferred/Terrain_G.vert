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

const float HPower = 3.0;

float GetWorldPosHeight(vec2 worldPos){
    return texture(_HEIGHT_MAP, (worldPos + _SIZE.xy * 0.5) / _SIZE.xy).r;
}

float GetCoarseHeight(vec2 worldPos){
    float cStep = _LOD_SCALE * 2.0; // next one is twice as big
    vec2 origin = floor(worldPos / cStep) * cStep;
    vec2 t = (worldPos - origin) / cStep;

    float h00 = GetWorldPosHeight(origin);
    float h10 = GetWorldPosHeight(origin + vec2(cStep, 0.0));
    float h01 = GetWorldPosHeight(origin + vec2(0.0, cStep));
    float h11 = GetWorldPosHeight(origin + vec2(cStep, cStep));

    // Barycentric interpolation matching the C++ index buffer diagonal
    if (t.x + t.y < 1.0) {
        // Upper-Left Triangle
        return h00 + t.x * (h10 - h00) + t.y * (h01 - h00);
    } else {
        // Bottom-Right Triangle
        return h11 + (1.0 - t.x) * (h01 - h11) + (1.0 - t.y) * (h10 - h11);
    }
}

void main(){
    float hScale = _SIZE.z;
    vec2 halfSize = _SIZE.xy * 0.5;
    float downOffset = pow(0.5 * hScale, HPower);

    vec3 flatPos = (MATRIX_MODEL * vec4(position.x, 0.0, position.y, 1.0)).xyz;
    v_uv = (flatPos.xz + halfSize) / _SIZE.xy;
    v_uv = fract(v_uv);

    // Calculate Alpha Morph factor based on continuous camera distance
    vec2 d = abs(flatPos.xz - _CAMERA_POSITION) / _LOD_SCALE;
    float dMax = max(d.x, d.y);

    // Morph region tightly constrained to the outer edge (24 to 30)
    float alpha = clamp((dMax - 24.0) / 6.0, 0.0, 1.0);

    float hRaw = texture(_HEIGHT_MAP, v_uv).r;
    float hCoarsed = GetCoarseHeight(flatPos.xz);
    float hBlendedRaw = mix(hRaw, hCoarsed, alpha);
    float h = pow(hBlendedRaw * hScale, HPower) - downOffset;


    vec3 localPos = vec3(position.x, h, position.y);
    FragPos = (MATRIX_MODEL * vec4(localPos, 1.0)).xyz;
    gl_Position = (MATRIX_PROJECTION * MATRIX_VIEW * MATRIX_MODEL) * vec4(localPos, 1.0);

    // Step by _LOD_SCALE instead of GridStep to prevent aliasing
    vec2 rightXZ = flatPos.xz + vec2(_LOD_SCALE, 0.0);
    vec2 upXZ    = flatPos.xz + vec2(0.0, _LOD_SCALE);

    float hRightRaw = mix(GetWorldPosHeight(rightXZ), GetCoarseHeight(rightXZ), alpha);
    float hUpRaw    = mix(GetWorldPosHeight(upXZ), GetCoarseHeight(upXZ), alpha);

    float hRight = pow(hRightRaw * hScale, HPower) - downOffset;
    float hUp    = pow(hUpRaw * hScale, HPower) - downOffset;

    // Calculate Tangents and Normals directly in WORLD space
    vec3 worldTangent   = normalize(vec3(_LOD_SCALE, hRight - h, 0.0));
    vec3 worldBitangent = normalize(vec3(0.0, hUp - h, _LOD_SCALE));
    vec3 worldNormal    = normalize(cross(worldBitangent, worldTangent));

    // Assign directly, skipping normalMatrix completely!
    out_normal = worldNormal;

// TBN Matrix directly from world vectors
//    vec3 T = worldTangent;
//    vec3 N = worldNormal;
//    T = normalize(T - dot(T, N) * N);
//    vec3 B = cross(N, T);
//    out_TBN = mat3(T, B, N);
    out_vertColor = vec4(_COLOR_TINT.xyz, 1.0);
}