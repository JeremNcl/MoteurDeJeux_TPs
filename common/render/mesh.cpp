#include "mesh.hpp"
#include "../io/objectLoader.hpp"

#include <cmath>
#include <iostream>

#include <glm/gtc/constants.hpp>


// === CACHE DE MESHES ===
std::unordered_map<std::string, std::shared_ptr<Mesh>> Mesh::meshCache;


// === CONSTRUCTEURS ===

Mesh::Mesh(){}

std::shared_ptr<Mesh> Mesh::generateSphere(float radius, int meridianCount, int parallelCount) {
    
    auto mesh = std::make_shared<Mesh>();
    mesh->vertices.clear();
    mesh->uvs.clear();
    mesh->indices.clear();
    mesh->normals.clear();

    for (int j = 0; j <= parallelCount; ++j) {
        float v = float(j) / float(parallelCount);
        float theta = v * glm::pi<float>();
        for (int i = 0; i <= meridianCount; ++i) {
            float u = float(i) / float(meridianCount);
            float phi = u * 2.0f * glm::pi<float>();
            float x = radius * std::sin(theta) * std::cos(phi);
            float y = radius * std::cos(theta);
            float z = radius * std::sin(theta) * std::sin(phi);
            mesh->vertices.emplace_back(x, y, z);
            mesh->uvs.emplace_back(u, 1.0f - v); 
            mesh->normals.emplace_back(glm::normalize(glm::vec3(x, y, z)));
        }
    }

    for (int j = 0; j < parallelCount; ++j) {
        for (int i = 0; i < meridianCount; ++i) {
            int curr = j * (meridianCount + 1) + i;
            int next = (j + 1) * (meridianCount + 1) + i;
            mesh->indices.push_back(curr);
            mesh->indices.push_back(next);
            mesh->indices.push_back(curr + 1);

            mesh->indices.push_back(curr + 1);
            mesh->indices.push_back(next);
            mesh->indices.push_back(next + 1);
        }
    }

    mesh->setMeridianCount(meridianCount);
    mesh->setParallelCount(parallelCount);
    return mesh;
}

// Chargement depuis fichier OFF (avec cache optionnel)
std::shared_ptr<Mesh> Mesh::loadFromOFF(const std::string& filename, bool enableCache) {
    
    if (enableCache) {
        auto it = meshCache.find(filename);
        if (it != meshCache.end()) {
            std::cout << "Cache hit: " << filename << std::endl;
            return it->second;
        }
    }

    auto mesh = std::make_shared<Mesh>();
    std::vector<std::vector<unsigned int>> triangles;

    if (!loadOFF(filename, mesh->vertices, mesh->indices, triangles)) {
        std::cerr << "ERREUR: Impossible de charger " << filename << std::endl;
        return nullptr;
    }

    if (enableCache) {
        meshCache[filename] = mesh;
    }

    std::cout << "Mesh chargé: " << filename
              << " (" << mesh->vertices.size() << " vertices, "
              << mesh->indices.size() << " indices)" << std::endl;

    return mesh;
}

// Chargement depuis un ficher OBJ
std::shared_ptr<Mesh> Mesh::loadFromOBJ(const std::string& filename, bool enableCache) {
    if (enableCache) {
        auto it = meshCache.find(filename);
        if (it != meshCache.end()) {
            std::cout << "Cache hit (OBJ): " << filename << std::endl;
            return it->second;
        }
    }

    auto mesh = std::make_shared<Mesh>();

    if (!loadOBJ(filename, mesh->vertices, mesh->uvs, mesh->normals, mesh->indices)) {
        std::cerr << "ERREUR: Impossible de charger " << filename << std::endl;
        return nullptr;
    }

    // Si le fichier OBJ n'avait pas de normales ou d'UVs, on peut les calculer
    if (mesh->normals.empty()) {
        mesh->computeNormals();
    }
    if (mesh->uvs.empty()) {
        mesh->computeUVs();
    }

    if (enableCache) {
        meshCache[filename] = mesh;
    }

    std::cout << "Mesh OBJ chargé: " << filename
              << " (" << mesh->vertices.size() << " vertices)" << std::endl;

    return mesh;
}

// === DESTRUCTEUR ===

Mesh::~Mesh(){}


// === CALCUL DES NORMALES ET UVs ===

void Mesh::computeNormals()
{
    normals.assign(vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i + 2 < indices.size(); i += 3) {
        unsigned int i0 = indices[i];
        unsigned int i1 = indices[i + 1];
        unsigned int i2 = indices[i + 2];

        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) {
            continue;
        }

        glm::vec3 e1 = vertices[i1] - vertices[i0];
        glm::vec3 e2 = vertices[i2] - vertices[i0];
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
    uvs.reserve(vertices.size());

    for (const auto& vertex : vertices) {
        float r = glm::length(vertex);
        glm::vec3 n = (r > 0.0f) ? (vertex / r) : glm::vec3(0.0f, 1.0f, 0.0f);

        float u = 0.5f + std::atan2(n.z, n.x) / (2.0f * glm::pi<float>());
        float v = 0.5f - std::asin(glm::clamp(n.y, -1.0f, 1.0f)) / glm::pi<float>();
        uvs.emplace_back(u, v);
    }
}


// === GESTION DU CACHE DE MESHES ===

void Mesh::clearMeshCache() {
    std::cout << "Vidage du cache de meshes (" << meshCache.size() << " entrées)" << std::endl;
    meshCache.clear();
}