#include "engine/mesh.h"

#include <glm/gtc/constants.hpp>
#include <cmath>

Mesh createMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    Mesh mesh;
    mesh.indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    return mesh;
}

Mesh createGroundMesh() {
    std::vector<float> groundVertices = {
        -15.0f, -0.01f, -15.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
         15.0f, -0.01f, -15.0f, 0.0f, 1.0f, 0.0f, 8.0f, 0.0f,
         15.0f, -0.01f,  15.0f, 0.0f, 1.0f, 0.0f, 8.0f, 8.0f,
        -15.0f, -0.01f,  15.0f, 0.0f, 1.0f, 0.0f, 0.0f, 8.0f
    };
    std::vector<unsigned int> groundIndices = { 0, 1, 2, 2, 3, 0 };

    return createMesh(groundVertices, groundIndices);
}

Mesh createSphereMesh(float radius, unsigned int sectorCount, unsigned int stackCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i <= stackCount; ++i) {
        float stackAngle = glm::pi<float>() / 2 - i * glm::pi<float>() / stackCount;
        float xy = radius * std::cos(stackAngle);
        float z = radius * std::sin(stackAngle);

        for (unsigned int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * glm::pi<float>() / sectorCount;
            float x = xy * std::cos(sectorAngle);
            float y = xy * std::sin(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            vertices.push_back(x / radius);
            vertices.push_back(y / radius);
            vertices.push_back(z / radius);
            vertices.push_back((float)j / sectorCount);
            vertices.push_back((float)i / stackCount);
        }
    }

    for (unsigned int i = 0; i < stackCount; ++i) {
        unsigned int k1 = i * (sectorCount + 1);
        unsigned int k2 = k1 + sectorCount + 1;

        for (unsigned int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    return createMesh(vertices, indices);
}

void drawMesh(const Mesh& mesh) {
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0);
}

void destroyMesh(Mesh& mesh) {
    if (mesh.VAO) glDeleteVertexArrays(1, &mesh.VAO);
    if (mesh.VBO) glDeleteBuffers(1, &mesh.VBO);
    if (mesh.EBO) glDeleteBuffers(1, &mesh.EBO);

    mesh.VAO = 0;
    mesh.VBO = 0;
    mesh.EBO = 0;
    mesh.indexCount = 0;
}
