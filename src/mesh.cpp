#include "engine/Mesh.h"

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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    return mesh;
}

Mesh createGroundMesh() {
    std::vector<float> groundVertices = {
        -15.0f, -0.01f, -15.0f, 0.0f, 0.0f,
         15.0f, -0.01f, -15.0f, 8.0f, 0.0f,
         15.0f, -0.01f,  15.0f, 8.0f, 8.0f,
        -15.0f, -0.01f,  15.0f, 0.0f, 8.0f
    };
    std::vector<unsigned int> groundIndices = { 0, 1, 2, 2, 3, 0 };

    return createMesh(groundVertices, groundIndices);
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
