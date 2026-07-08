#include "Mesh.h"
#include <cmath>

void ClearMesh(Mesh& mesh) {
    if (mesh.vertexData) {
        delete[] mesh.vertexData;
        mesh.vertexData = nullptr;
    }
    if (mesh.indexData) {
        delete[] mesh.indexData;
        mesh.indexData = nullptr;
    }
    mesh.vertexDataCount = 0;
    mesh.indexCount = 0;
}

void Mesh_DefaultSphere(Mesh& mesh) {
    constexpr int segments = 32;
    constexpr int rings = 16;
    mesh.stride = 8;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexDataCount = (rings + 1) * (segments + 1) * mesh.stride;
    mesh.indexCount = rings * segments * 6;
    mesh.vertexData = new float[mesh.vertexDataCount];
    mesh.indexData = new int[mesh.indexCount];
    int vIdx = 0;
    for (int y = 0; y <= rings; y++) {
        float v = (float)y / rings;
        float phi = v * 3.14159265359f;
        for (int x = 0; x <= segments; x++) {
            float u = (float)x / segments;
            float theta = u * 2.0f * 3.14159265359f;
            float nx = std::cos(theta) * std::sin(phi);
            float ny = std::cos(phi);
            float nz = std::sin(theta) * std::sin(phi);
            mesh.vertexData[vIdx++] = nx * 0.5f;
            mesh.vertexData[vIdx++] = ny * 0.5f;
            mesh.vertexData[vIdx++] = nz * 0.5f;
            mesh.vertexData[vIdx++] = u;
            mesh.vertexData[vIdx++] = v;
            mesh.vertexData[vIdx++] = nx;
            mesh.vertexData[vIdx++] = ny;
            mesh.vertexData[vIdx++] = nz;
        }
    }
    int iIdx = 0;
    for (int y = 0; y < rings; y++) {
        for (int x = 0; x < segments; x++) {
            int p0 = y * (segments + 1) + x;
            int p1 = p0 + 1;
            int p2 = p0 + (segments + 1);
            int p3 = p2 + 1;
            mesh.indexData[iIdx++] = p1;
            mesh.indexData[iIdx++] = p2;
            mesh.indexData[iIdx++] = p0;

            mesh.indexData[iIdx++] = p3;
            mesh.indexData[iIdx++] = p2;
            mesh.indexData[iIdx++] = p1;
        }
    }
}

void Mesh_DefaultCapsule(Mesh& mesh) {
    constexpr int segments = 32;
    constexpr int rings = 16;
    constexpr int halfRings = rings / 2;
    constexpr float radius = 0.5f;
    constexpr float height = 2.0f;
    constexpr float cylinderHeight = height - 2.0f * radius;
    mesh.stride = 8;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexDataCount = ((halfRings + 1) * 2 * (segments + 1)) * mesh.stride;
    mesh.indexCount = (rings + 1) * segments * 6;
    mesh.vertexData = new float[mesh.vertexDataCount];
    mesh.indexData = new int[mesh.indexCount];
    int vIdx = 0;
    for (int y = 0; y <= halfRings; y++) {
        float phi = ((float)y / halfRings) * 1.57079632679f;
        for (int x = 0; x <= segments; x++) {
            float u = (float)x / segments;
            float theta = u * 2.0f * 3.14159265359f;
            float nx = std::cos(theta) * std::sin(phi);
            float ny = std::cos(phi);
            float nz = std::sin(theta) * std::sin(phi);
            float trueY = ny * radius + cylinderHeight * 0.5f;
            float v = 1.0f - (trueY + height * 0.5f) / height;
            mesh.vertexData[vIdx++] = nx * radius;
            mesh.vertexData[vIdx++] = trueY;
            mesh.vertexData[vIdx++] = nz * radius;
            mesh.vertexData[vIdx++] = u;
            mesh.vertexData[vIdx++] = v;
            mesh.vertexData[vIdx++] = nx;
            mesh.vertexData[vIdx++] = ny;
            mesh.vertexData[vIdx++] = nz;
        }
    }
    for (int y = 0; y <= halfRings; y++) {
        float phi = 1.57079632679f + ((float)y / halfRings) * 1.57079632679f;
        for (int x = 0; x <= segments; x++) {
            float u = (float)x / segments;
            float theta = u * 2.0f * 3.14159265359f;
            float nx = std::cos(theta) * std::sin(phi);
            float ny = std::cos(phi);
            float nz = std::sin(theta) * std::sin(phi);
            float trueY = ny * radius - cylinderHeight * 0.5f;
            float v = 1.0f - (trueY + height * 0.5f) / height;
            mesh.vertexData[vIdx++] = nx * radius;
            mesh.vertexData[vIdx++] = trueY;
            mesh.vertexData[vIdx++] = nz * radius;
            mesh.vertexData[vIdx++] = u;
            mesh.vertexData[vIdx++] = v;
            mesh.vertexData[vIdx++] = nx;
            mesh.vertexData[vIdx++] = ny;
            mesh.vertexData[vIdx++] = nz;
        }
    }
    int iIdx = 0;
    for (int y = 0; y <= rings; y++) {
        for (int x = 0; x < segments; x++) {
            int p0 = y * (segments + 1) + x;
            int p1 = p0 + 1;
            int p2 = p0 + (segments + 1);
            int p3 = p2 + 1;
            mesh.indexData[iIdx++] = p0;
            mesh.indexData[iIdx++] = p1;
            mesh.indexData[iIdx++] = p2;

            mesh.indexData[iIdx++] = p1;
            mesh.indexData[iIdx++] = p3;
            mesh.indexData[iIdx++] = p2;
        }
    }
}


void Mesh_DefaultPyramid(Mesh& mesh) {
    mesh.stride = 8;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexDataCount = 16 * mesh.stride;
    mesh.indexCount = 18;

    mesh.vertexData = new float[] {
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, 0.4472136f, 0.8944272f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, 0.4472136f, 0.8944272f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,   0.0f, 0.4472136f, 0.8944272f,
        // Right Face (+X)
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.8944272f, 0.4472136f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.8944272f, 0.4472136f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,   0.8944272f, 0.4472136f, 0.0f,
        // Back Face (-Z)
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f,   0.0f, 0.4472136f, -0.8944272f,
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f,   0.0f, 0.4472136f, -0.8944272f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,   0.0f, 0.4472136f, -0.8944272f,
        // Left Face (-X)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  -0.8944272f, 0.4472136f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  -0.8944272f, 0.4472136f, 0.0f,
         0.0f,  0.5f,  0.0f,  0.5f, 1.0f,  -0.8944272f, 0.4472136f, 0.0f,
        // Bottom Face (-Y)
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,   0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,   0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,   0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,   0.0f, -1.0f, 0.0f
    };

    mesh.indexData = new int[] {
        0, 1, 2,       // Front
        3, 4, 5,       // Right
        6, 7, 8,       // Back
        9, 10, 11,     // Left
        12, 13, 14,    // Bottom Triangle 1
        12, 14, 15     // Bottom Triangle 2
    };
}


void Mesh_DefaultPlane(Mesh& mesh) {
    Mesh_DefaultQuad(mesh);
}

void Mesh_DefaultQuad(Mesh& mesh) {
    mesh.stride = 8;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexDataCount = 4 * mesh.stride;
    mesh.indexCount = 6;
    mesh.vertexData = new float[] {
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f,   0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f,   0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f,   0.0f, 0.0f, 1.0f
    };
    mesh.indexData = new int[] {
        0, 1, 2, 0, 2, 3
    };
}

void Mesh_DefaultDonut(Mesh& mesh) {
    constexpr int mainSegments = 32;
    constexpr int tubeSegments = 16;
    constexpr float majorRadius = 0.75f;
    constexpr float minorRadius = 0.25f;
    mesh.stride = 8;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexDataCount = (mainSegments + 1) * (tubeSegments + 1) * mesh.stride;
    mesh.indexCount = mainSegments * tubeSegments * 6;
    mesh.vertexData = new float[mesh.vertexDataCount];
    mesh.indexData = new int[mesh.indexCount];
    constexpr float PI = 3.14159265359f;
    constexpr float TwoPI = 2 * PI;

    int vIdx = 0;
    for (int y = 0; y <= mainSegments; y++) {
        float u = (float)y / mainSegments;
        float theta = u * TwoPI;
        float cosTheta = std::cos(theta);
        float sinTheta = std::sin(theta);
        for (int x = 0; x <= tubeSegments; x++) {
            float v = (float)x / tubeSegments;
            float phi = v * TwoPI;
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);
            float cx = majorRadius * cosTheta;
            float cy = 0.0f;
            float cz = majorRadius * sinTheta;
            float px = (majorRadius + minorRadius * cosPhi) * cosTheta;
            float py = minorRadius * sinPhi;
            float pz = (majorRadius + minorRadius * cosPhi) * sinTheta;
            float nx = px - cx;
            float ny = py - cy;
            float nz = pz - cz;
            float invLen = 1.0f / std::sqrt(nx * nx + ny * ny + nz * nz);
            nx *= invLen;
            ny *= invLen;
            nz *= invLen;
            mesh.vertexData[vIdx++] = px;
            mesh.vertexData[vIdx++] = py;
            mesh.vertexData[vIdx++] = pz;
            mesh.vertexData[vIdx++] = u;
            mesh.vertexData[vIdx++] = v;
            mesh.vertexData[vIdx++] = nx;
            mesh.vertexData[vIdx++] = ny;
            mesh.vertexData[vIdx++] = nz;
        }
    }
    int iIdx = 0;
    for (int y = 0; y < mainSegments; y++) {
        for (int x = 0; x < tubeSegments; x++) {
            int p0 = y * (tubeSegments + 1) + x;
            int p1 = p0 + 1;
            int p2 = p0 + (tubeSegments + 1);
            int p3 = p2 + 1;
            mesh.indexData[iIdx++] = p0;
            mesh.indexData[iIdx++] = p2;
            mesh.indexData[iIdx++] = p1;
            mesh.indexData[iIdx++] = p1;
            mesh.indexData[iIdx++] = p2;
            mesh.indexData[iIdx++] = p3;
        }
    }
}
void Mesh_DefaultCutCone(Mesh& mesh)
{
    constexpr float d = 0.5f;
    constexpr float ds = 0.25f;
    mesh.stride = 8;
    mesh.vertexDataCount = 24 * mesh.stride;
    mesh.indexCount = 36;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexData = new float[] {
        // POSITION    // UV       // Normal
        // Front Face (Z = 0.5f)
        -ds, -ds,  d,   0.0f, 0.0f,   0, 0,  1,
         ds, -ds,  d,   1.0f, 0.0f,   0, 0,  1,
         ds,  ds,  d,   1.0f, 1.0f,   0, 0,  1,
        -ds,  ds,  d,   0.0f, 1.0f,   0, 0,  1,
        // Back Face (Z = -d)
         d, -d, -d,   0.0f, 0.0f,   0, 0, -1,
        -d, -d, -d,   1.0f, 0.0f,   0, 0, -1,
        -d,  d, -d,   1.0f, 1.0f,   0, 0, -1,
         d,  d, -d,   0.0f, 1.0f,   0, 0, -1,
        // Left Face (X = -d)
        -d, -d, -d,   0.0f, 0.0f, - 1, 0, 0,
        -ds, -ds, d,   1.0f, 0.0f, - 1, 0, 0,
        -ds,  ds, d,   1.0f, 1.0f, - 1, 0, 0,
        -d,  d, -d,   0.0f, 1.0f, - 1, 0, 0,
        // Right Face (X = d)
         ds, -ds,  d,   0.0f, 0.0f,   1, 0, 0,
         d, -d, -d,   1.0f, 0.0f,   1, 0, 0,
         d,  d, -d,   1.0f, 1.0f,   1, 0, 0,
         ds,  ds, d,   0.0f, 1.0f,   1, 0, 0,
        // Top Face (Y = d)
        -ds,  ds,  d,   0.0f, 0.0f,   0, 1, 0,
         ds,  ds,  d,   1.0f, 0.0f,   0, 1, 0,
         d,  d, -d,   1.0f, 1.0f,   0, 1, 0,
        -d,  d, -d,   0.0f, 1.0f,   0, 1, 0,
        // Bottom Face (Y = -d)
        -d, -d, -d,   0.0f, 0.0f,   0, -1, 0,
         d, -d, -d,   1.0f, 0.0f,   0, -1, 0,
         ds, -ds,  d,   1.0f, 1.0f,   0, -1, 0,
        -ds, -ds,  d,   0.0f, 1.0f,   0, -1, 0,
    };

    mesh.indexData = new int[mesh.indexCount] {
        0,  1,  2,    2,  3,  0,  // Front Face
        4,  5,  6,    6,  7,  4,  // Back Face
        8,  9,  10,   10, 11, 8,  // Left Face
        12, 13, 14,   14, 15, 12, // Right Face
        16, 17, 18,   18, 19, 16, // Top Face
        20, 21, 22,   22, 23, 20  // Bottom Face
    };
}

void Mesh_DefaultCube(Mesh& mesh) {
    constexpr float d = 0.5f;
    mesh.stride = 8;
    mesh.vertexDataCount = 24 * mesh.stride;
    mesh.indexCount = 36;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexData = new float[] {
        // POSITION    // UV       // Normal
        // Front Face (Z = 0.5f)
        -d, -d,  d,   0.0f, 0.0f,   0, 0,  1,
         d, -d,  d,   1.0f, 0.0f,   0, 0,  1,
         d,  d,  d,   1.0f, 1.0f,   0, 0,  1,
        -d,  d,  d,   0.0f, 1.0f,   0, 0,  1,
        // Back Face (Z = -d)
         d, -d, -d,   0.0f, 0.0f,   0, 0, -1,
        -d, -d, -d,   1.0f, 0.0f,   0, 0, -1,
        -d,  d, -d,   1.0f, 1.0f,   0, 0, -1,
         d,  d, -d,   0.0f, 1.0f,   0, 0, -1,
        // Left Face (X = -d)
        -d, -d, -d,   0.0f, 0.0f, - 1, 0, 0,
        -d, -d,  d,   1.0f, 0.0f, - 1, 0, 0,
        -d,  d,  d,   1.0f, 1.0f, - 1, 0, 0,
        -d,  d, -d,   0.0f, 1.0f, - 1, 0, 0,
        // Right Face (X = d)
         d, -d,  d,   0.0f, 0.0f,   1, 0, 0,
         d, -d, -d,   1.0f, 0.0f,   1, 0, 0,
         d,  d, -d,   1.0f, 1.0f,   1, 0, 0,
         d,  d,  d,   0.0f, 1.0f,   1, 0, 0,
        // Top Face (Y = d)
        -d,  d,  d,   0.0f, 0.0f,   0, 1, 0,
         d,  d,  d,   1.0f, 0.0f,   0, 1, 0,
         d,  d, -d,   1.0f, 1.0f,   0, 1, 0,
        -d,  d, -d,   0.0f, 1.0f,   0, 1, 0,
        // Bottom Face (Y = -d)
        -d, -d, -d,   0.0f, 0.0f,   0, -1, 0,
         d, -d, -d,   1.0f, 0.0f,   0, -1, 0,
         d, -d,  d,   1.0f, 1.0f,   0, -1, 0,
        -d, -d,  d,   0.0f, 1.0f,   0, -1, 0,
    };

    mesh.indexData = new int[mesh.indexCount] {
        0,  1,  2,    2,  3,  0,  // Front Face
        4,  5,  6,    6,  7,  4,  // Back Face
        8,  9,  10,   10, 11, 8,  // Left Face
        12, 13, 14,   14, 15, 12, // Right Face
        16, 17, 18,   18, 19, 16, // Top Face
        20, 21, 22,   22, 23, 20  // Bottom Face
    };
}

void Mesh_Print(Mesh& mesh) {
    printf("[m] vertexDataCount %d\n", mesh.vertexDataCount );
    printf("[m] indexCount %d\n", mesh.indexCount );
    printf("[m] stride %d\n", mesh.stride );
    printf("[m] startIndexVertex %d\n", mesh.startIndexVertex );
    printf("[m] startIndexUV %d\n", mesh.startIndexUV );
    printf("[m] startIndexColor %d\n", mesh.startIndexColor );
    printf("[m] startIndexNormals %d\n", mesh.startIndexNormals );

    for (size_t i = 0; i < mesh.vertexDataCount; i++) {
        if (i > 0 && i % 3 == 0) {
            printf("\n");
        }
        float vd = mesh.vertexData[i];
        printf("%f, ", vd);
    }
}
