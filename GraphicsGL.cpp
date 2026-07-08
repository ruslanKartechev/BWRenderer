#include "GraphicsGL.h"


void AllocateGraphicsForObject(RenderObject& obj, GameScene& scene)
{
    obj.meshData.reserve(1);
    if (obj.meshData.size() < 1) {
        obj.meshData.resize(1);
    }
    for (auto& meshRenderData : obj.meshData) {

        Mesh& mesh = scene.meshes.GetItemRef(meshRenderData.hMesh);

        GLuint vao;
        GLuint vbo;
        GLuint ebo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertexDataCount, mesh.vertexData, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * mesh.indexCount, mesh.indexData, GL_DYNAMIC_DRAW);

        int stride = mesh.stride * sizeof(float);
        int attributeIdx = 0;
        glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0); // XYZ
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
        if (mesh.startIndexUV >= 0) {
            glVertexAttribPointer(attributeIdx, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexUV*sizeof(float))); // UV
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }
        if (mesh.startIndexNormals >= 0) {
            glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexNormals*sizeof(float))); // XYZ normals
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }
        if (mesh.startIndexColor >= 0) {
            glVertexAttribPointer(attributeIdx, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }

        glBindVertexArray(0);
        meshRenderData.vao = vao;
        meshRenderData.vbo = vbo;
        meshRenderData.ebo = ebo;
    }
}
