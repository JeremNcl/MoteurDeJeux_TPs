#include "Mesh.hpp"
#include "../io/objloader.hpp"

#include <cmath>
#include <iostream>
#include <glm/gtc/constants.hpp>

std::unordered_map<std::string, std::shared_ptr<Mesh>> Mesh::meshCache;

Mesh::Mesh()
    : vertexBuffer(0)
    , indexBuffer(0)
    , uvBuffer(0)
    , normalBuffer(0)
    , indexCount(0)
    , texture(0)
    , shaderProgram(0)
{}

Mesh::~Mesh()
{
    releaseGPU();
}

std::shared_ptr<Mesh> Mesh::loadFromOFF(const std::string& filename, bool enableCache)
{
    if (enableCache) {
        auto it = meshCache.find(filename);
        if (it != meshCache.end()) {
            std::cout << "Cache hit: " << filename << std::endl;
            return it->second;
        }
    }

    auto mesh = std::make_shared<Mesh>();
    std::vector<std::vector<unsigned int>> triangles;

    if (!loadOFF(filename, mesh->positions, mesh->indices, triangles)) {
        std::cerr << "ERREUR: Impossible de charger " << filename << std::endl;
        return nullptr;
    }

    mesh->indexCount = mesh->indices.size();

    if (enableCache) {
        meshCache[filename] = mesh;
    }

    std::cout << "Mesh chargé: " << filename
              << " (" << mesh->positions.size() << " vertices, "
              << mesh->indices.size() << " indices)" << std::endl;

    return mesh;
}

void Mesh::clearMeshCache()
{
    std::cout << "Vidage du cache de meshes (" << meshCache.size() << " entrées)" << std::endl;
    meshCache.clear();
}

void Mesh::computeNormals()
{
    normals.assign(positions.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        if (i0 >= positions.size() || i1 >= positions.size() || i2 >= positions.size()) {
            continue;
        }

        glm::vec3 e1 = positions[i1] - positions[i0];
        glm::vec3 e2 = positions[i2] - positions[i0];
        glm::vec3 faceNormal = glm::cross(e1, e2);
        float length = glm::length(faceNormal);
        if (length > 0.0f) {
            faceNormal /= length;
            normals[i0] += faceNormal;
            normals[i1] += faceNormal;
            normals[i2] += faceNormal;
        }
    }

    for (auto& normal : normals) {
        float length = glm::length(normal);
        if (length > 0.0f) {
            normal /= length;
        } else {
            normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}

void Mesh::computeUVs()
{
    uvs.clear();
    uvs.reserve(positions.size());

    for (const auto& position : positions) {
        float r = glm::length(position);
        glm::vec3 n = (r > 0.0f) ? (position / r) : glm::vec3(0.0f, 1.0f, 0.0f);

        float u = 0.5f + std::atan2(n.z, n.x) / (2.0f * glm::pi<float>());
        float v = 0.5f - std::asin(glm::clamp(n.y, -1.0f, 1.0f)) / glm::pi<float>();
        uvs.emplace_back(u, v);
    }
}

void Mesh::uploadToGPU()
{
    if (positions.empty() || indices.empty()) {
        return;
    }

    if (vertexBuffer == 0) {
        glGenBuffers(1, &vertexBuffer);
    }
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);

    if (indexBuffer == 0) {
        glGenBuffers(1, &indexBuffer);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    if (!uvs.empty()) {
        if (uvBuffer == 0) {
            glGenBuffers(1, &uvBuffer);
        }
        glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
        glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), uvs.data(), GL_STATIC_DRAW);
    }

    if (!normals.empty()) {
        if (normalBuffer == 0) {
            glGenBuffers(1, &normalBuffer);
        }
        glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    }

    indexCount = indices.size();
}

void Mesh::releaseGPU()
{
    if (vertexBuffer != 0) {
        glDeleteBuffers(1, &vertexBuffer);
        vertexBuffer = 0;
    }
    if (indexBuffer != 0) {
        glDeleteBuffers(1, &indexBuffer);
        indexBuffer = 0;
    }
    if (uvBuffer != 0) {
        glDeleteBuffers(1, &uvBuffer);
        uvBuffer = 0;
    }
    if (normalBuffer != 0) {
        glDeleteBuffers(1, &normalBuffer);
        normalBuffer = 0;
    }
}

bool Mesh::hasGPUData() const
{
    return vertexBuffer != 0 && indexBuffer != 0 && indexCount > 0;
}
