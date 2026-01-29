#include "terrain.hpp"
#include "heightmap_loader.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>

Terrain::Terrain() 
    : width(16), height(16), maxHeight(50.0f) {}

bool Terrain::loadHeightmap(const std::string& filepath, float maxHeight) {
    this->maxHeight = maxHeight;
    
    int hmWidth, hmHeight;
    heightmap = ::loadHeightmap(filepath.c_str(), hmWidth, hmHeight);
    
    if (heightmap.empty()) {
        printf("Failed to load heightmap from %s, using flat plane\n", filepath.c_str());
        width = 16;
        height = 16;
        return false;
    }
    
    width = hmWidth;
    height = hmHeight;
    printf("Heightmap loaded: %dx%d\n", width, height);
    return true;
}

void Terrain::generateFlatPlane(int w, int h) {
    width = w;
    height = h;
    heightmap.clear();
    printf("Generated flat plane: %dx%d\n", width, height);
}

void Terrain::generateMesh(std::vector<glm::vec3>& vertices, 
                           std::vector<unsigned int>& indices,
                           std::vector<glm::vec2>& uvs,
                           std::vector<glm::vec3>& normals) {
    vertices.clear();
    indices.clear();
    uvs.clear();
    normals.clear();

    // Création des sommets et des uvs
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            glm::vec3 v;
            v.x = (float)x;
            
            // Utiliser la heightmap pour la hauteur
            if (!heightmap.empty() && z < (int)heightmap.size() && x < (int)heightmap[z].size()) {
                v.y = heightmap[z][x] * maxHeight;
            } else {
                v.y = 0.0f;
            }
            
            v.z = (float)z;
            vertices.push_back(v);
            
            glm::vec2 uv;
            uv.x = x / float(width - 1);
            uv.y = z / float(height - 1);
            uvs.push_back(uv);
            
            // Initialiser les normales à zéro (seront calculées après)
            normals.push_back(glm::vec3(0.0f, 0.0f, 0.0f));
        }
    }

    // Indexation des triangles
    for (unsigned int j = 0; j < (unsigned int)height; j++) {
        for (unsigned int i = 0; i < (unsigned int)width; i++) {
            if (i+1 < (unsigned int)width && j+1 < (unsigned int)height) {
                // Triangle 1
                indices.push_back(i + j*width);       // TOP_LEFT
                indices.push_back(i + (j+1)*width);   // BOTTOM_LEFT
                indices.push_back(i+1 + j*width);     // TOP_RIGHT
                // Triangle 2
                indices.push_back(i+1 + j*width);     // TOP_RIGHT
                indices.push_back(i + (j+1)*width);   // BOTTOM_LEFT
                indices.push_back(i+1 + (j+1)*width); // BOTTOM_RIGHT
            }
        }
    }
    
    // Calculer les normales
    calculateNormals(vertices, indices, normals);
    
    printf("Mesh generated: %zu vertices, %zu indices\n", vertices.size(), indices.size());
}

void Terrain::calculateNormals(const std::vector<glm::vec3>& vertices,
                                const std::vector<unsigned int>& indices,
                                std::vector<glm::vec3>& normals) {
    // Pour chaque triangle, calculer la normale et l'ajouter aux normales des sommets
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int id0 = indices[i];
        unsigned int id1 = indices[i + 1];
        unsigned int id2 = indices[i + 2];
        
        glm::vec3 v0 = vertices[id0];
        glm::vec3 v1 = vertices[id1];
        glm::vec3 v2 = vertices[id2];
        
        // Calculer les vecteurs des arêtes du triangle
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        
        // Normale du triangle (produit vectoriel)
        glm::vec3 triangleNormal = glm::normalize(glm::cross(edge1, edge2));
        
        // Ajouter cette normale aux trois sommets du triangle
        normals[id0] += triangleNormal;
        normals[id1] += triangleNormal;
        normals[id2] += triangleNormal;
    }
    
    // Normaliser toutes les normales des sommets
    for (size_t i = 0; i < normals.size(); i++) {
        normals[i] = glm::normalize(normals[i]);
    }
    
    printf("Normals calculated for %zu vertices\n", normals.size());
}

glm::vec3 Terrain::getCenterPosition() const {
    return glm::vec3(width / 2.0f, 0.0f, height / 2.0f);
}

CameraSetup Terrain::getOptimalIsometricView() const {
    CameraSetup setup;
    
    // Centre du terrain
    float centerX = width / 2.0f;
    float centerZ = height / 2.0f;
    
    // Distance de la caméra pour voir tout le terrain
    // On utilise la plus grande dimension et on ajoute une marge
    float maxDim = std::max(width, height);
    float distance = maxDim * 1.2f;  // Distance augmentée pour voir tout le terrain
    
    // Position de la caméra en vue isométrique (45° en horizontal et en vertical)
    setup.position = glm::vec3(
        centerX + distance * 0.7f,  // Offset en X
        distance * 0.6f,             // Hauteur augmentée
        centerZ + distance * 0.7f    // Offset en Z
    );
    
    // La caméra regarde le centre du terrain
    setup.target = glm::vec3(centerX, 0.0f, centerZ);
    
    // Vecteur up standard
    setup.up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Vitesse de déplacement adaptée à la taille du terrain
    // Plus le terrain est grand, plus la vitesse doit être élevée
    setup.speed = maxDim * 0.3f;  // 30% de la dimension max par seconde
    
    printf("Optimal isometric view for terrain %dx%d\n", width, height);
    printf("  Position: (%.1f, %.1f, %.1f)\n", setup.position.x, setup.position.y, setup.position.z);
    printf("  Target: (%.1f, %.1f, %.1f)\n", setup.target.x, setup.target.y, setup.target.z);
    
    return setup;
}
