#include "engine/model.h"

#include "tinyobjloader/tiny_obj_loader.h"

#include <algorithm>
#include <cfloat>
#include <iostream>
#include <map>
#include <tuple>

struct ObjVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;

    bool operator<(const ObjVertex& other) const {
        return std::tie(pos.x, pos.y, pos.z, normal.x, normal.y, normal.z, uv.x, uv.y) <
               std::tie(other.pos.x, other.pos.y, other.pos.z, other.normal.x, other.normal.y, other.normal.z, other.uv.x, other.uv.y);
    }
};

LoadedModel loadObjModel(const std::string& objPath, const std::string& materialBaseDir) {
    LoadedModel model;
    std::map<ObjVertex, unsigned int> uniqueVertices;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, objPath.c_str(), materialBaseDir.c_str());
    if (!err.empty()) {
        std::cerr << "TinyOBJLoader: " << err << std::endl;
    }
    if (!ret) {
        std::cerr << "Failed to load OBJ: " << objPath << "\n";
        return model;
    }

    for (const auto& shape : shapes) {
        for (const auto& ind : shape.mesh.indices) {
            glm::vec3 position(
                attrib.vertices[3 * ind.vertex_index + 0],
                attrib.vertices[3 * ind.vertex_index + 1],
                attrib.vertices[3 * ind.vertex_index + 2]
            );

            glm::vec2 uv(0.0f, 0.0f);
            if (ind.texcoord_index >= 0) {
                uv.x = attrib.texcoords[2 * ind.texcoord_index + 0];
                uv.y = 1.0f - attrib.texcoords[2 * ind.texcoord_index + 1];
            }

            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            if (ind.normal_index >= 0) {
                normal.x = attrib.normals[3 * ind.normal_index + 0];
                normal.y = attrib.normals[3 * ind.normal_index + 1];
                normal.z = attrib.normals[3 * ind.normal_index + 2];
            }

            ObjVertex vertex{ position, normal, uv };
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<unsigned int>(model.vertices.size() / 8);
                model.vertices.push_back(position.x);
                model.vertices.push_back(position.y);
                model.vertices.push_back(position.z);
                model.vertices.push_back(normal.x);
                model.vertices.push_back(normal.y);
                model.vertices.push_back(normal.z);
                model.vertices.push_back(uv.x);
                model.vertices.push_back(uv.y);
            }

            model.indices.push_back(uniqueVertices[vertex]);
        }
    }

    std::cout << "Loaded vertices: " << model.vertices.size() / 8
              << ", indices: " << model.indices.size() << "\n";

    model.minBounds = glm::vec3(FLT_MAX);
    model.maxBounds = glm::vec3(-FLT_MAX);

    for (size_t i = 0; i < model.vertices.size(); i += 8) {
        model.minBounds.x = std::min(model.minBounds.x, model.vertices[i + 0]);
        model.minBounds.y = std::min(model.minBounds.y, model.vertices[i + 1]);
        model.minBounds.z = std::min(model.minBounds.z, model.vertices[i + 2]);
        model.maxBounds.x = std::max(model.maxBounds.x, model.vertices[i + 0]);
        model.maxBounds.y = std::max(model.maxBounds.y, model.vertices[i + 1]);
        model.maxBounds.z = std::max(model.maxBounds.z, model.vertices[i + 2]);
    }

    model.center = (model.minBounds + model.maxBounds) * 0.5f;
    glm::vec3 diff = model.maxBounds - model.minBounds;
    float maxDim = std::max({ diff.x, diff.y, diff.z });
    model.scale = maxDim > 0.0f ? 2.0f / maxDim : 1.0f;

    return model;
}
