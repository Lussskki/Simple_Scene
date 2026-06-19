#pragma once

#include <glm/glm.hpp>

#include <string>
#include <vector>

struct LoadedModel {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    glm::vec3 minBounds = glm::vec3(0.0f);
    glm::vec3 maxBounds = glm::vec3(0.0f);
    glm::vec3 center = glm::vec3(0.0f);
    float scale = 1.0f;
};

LoadedModel loadObjModel(const std::string& objPath, const std::string& materialBaseDir);
