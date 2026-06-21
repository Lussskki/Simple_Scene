#pragma once

#include <glad/glad.h>

#include <vector>

struct Mesh {
    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;
    GLsizei indexCount = 0;
};

Mesh createMesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);
Mesh createGroundMesh();
Mesh createSphereMesh(float radius, unsigned int sectorCount, unsigned int stackCount);
Mesh createWallMesh(float x, float y, float z);
void drawMesh(const Mesh& mesh);
void destroyMesh(Mesh& mesh);
