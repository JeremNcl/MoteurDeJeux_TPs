#include "terrain.hpp"
#include "heightmap_loader.hpp"
#include <cmath>
#include <algorithm>
#include <cstdio>

namespace {
int nextPowerOfTwoPlusOne(int value) {
    int pow2 = 1;
    while (pow2 + 1 < value) {
        pow2 *= 2;
    }
    return pow2 + 1;
}

float sampleHeightBilinear(const std::vector<std::vector<float>>& hm, int hmWidth, int hmHeight, float x, float z) {
    if (hm.empty() || hmWidth <= 0 || hmHeight <= 0) {
        return 0.0f;
    }

    if (x < 0.0f) x = 0.0f;
    if (z < 0.0f) z = 0.0f;
    if (x > hmWidth - 1) x = float(hmWidth - 1);
    if (z > hmHeight - 1) z = float(hmHeight - 1);

    int x0 = int(x);
    int z0 = int(z);
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    if (x1 >= hmWidth) x1 = hmWidth - 1;
    if (z1 >= hmHeight) z1 = hmHeight - 1;

    float fx = x - x0;
    float fz = z - z0;

    float h00 = (z0 < (int)hm.size() && x0 < (int)hm[z0].size()) ? hm[z0][x0] : 0.0f;
    float h10 = (z0 < (int)hm.size() && x1 < (int)hm[z0].size()) ? hm[z0][x1] : 0.0f;
    float h01 = (z1 < (int)hm.size() && x0 < (int)hm[z1].size()) ? hm[z1][x0] : 0.0f;
    float h11 = (z1 < (int)hm.size() && x1 < (int)hm[z1].size()) ? hm[z1][x1] : 0.0f;

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    return h0 * (1.0f - fz) + h1 * fz;
}
}

Terrain::Terrain() 
    : width(16), height(16), maxHeight(50.0f), resolution(1.0f) {}

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

    int targetSize = nextPowerOfTwoPlusOne(std::max(width, height));
    if (targetSize != width || targetSize != height) {
        std::vector<std::vector<float>> resized(targetSize, std::vector<float>(targetSize, 0.0f));
        float scaleX = (width - 1) / float(targetSize - 1);
        float scaleZ = (height - 1) / float(targetSize - 1);

        for (int z = 0; z < targetSize; ++z) {
            float srcZ = z * scaleZ;
            for (int x = 0; x < targetSize; ++x) {
                float srcX = x * scaleX;
                resized[z][x] = sampleHeightBilinear(heightmap, width, height, srcX, srcZ);
            }
        }

        heightmap.swap(resized);
        width = targetSize;
        height = targetSize;
        printf("Heightmap resized to: %dx%d (2^n + 1)\n", width, height);
    }
    return true;
}

void Terrain::generateFlatPlane(int w, int h) {
    width = w;
    height = h;
    heightmap.clear();
    printf("Generated flat plane: %dx%d\n", width, height);
}

void Terrain::setResolution(float step) {
    // Contraindre aux puissances de 2 : 0.25, 0.5, 1, 2, 4, 8, 16, ...
    float minStep = 0.25f;
    float maxStep = (width - 1) / 4.0f;  // Au minimum 4 quads par dimension
    
    // Snap à la puissance de 2 la plus proche
    if (step < minStep) step = minStep;
    if (step > maxStep) step = maxStep;
    
    // Trouver la puissance de 2 la plus proche
    float logStep = std::log2(step);
    float rounded = std::round(logStep);
    step = std::pow(2.0f, rounded);
    
    // Re-vérifier les bornes après snap
    if (step < minStep) step = minStep;
    if (step > maxStep) step = maxStep;
    
    resolution = step;
    int effectiveWidth = (int)std::round((width - 1) / resolution) + 1;
    int effectiveHeight = (int)std::round((height - 1) / resolution) + 1;
    printf("Résolution changée: step = %.2f (taille effective: %dx%d)\n", 
           resolution, effectiveWidth, effectiveHeight);
}

void Terrain::generateMesh(std::vector<glm::vec3>& vertices, 
                           std::vector<unsigned int>& indices,
                           std::vector<glm::vec2>& uvs,
                           std::vector<glm::vec3>& normals) {
    vertices.clear();
    indices.clear();
    uvs.clear();
    normals.clear();

    // Calcul de la taille effective du mesh
    int actualWidth = (int)std::round((width - 1) / resolution) + 1;
    int actualHeight = (int)std::round((height - 1) / resolution) + 1;

    // Création des sommets et des uvs avec résolution adaptée
    for (int j = 0; j < actualHeight; ++j) {
        float z = j * resolution;
        if (z > height - 1) z = float(height - 1);
        for (int i = 0; i < actualWidth; ++i) {
            float x = i * resolution;
            if (x > width - 1) x = float(width - 1);
            glm::vec3 v;
            v.x = x;
            
            // Interpoler la hauteur en fonction de la résolution
            v.y = sampleHeightBilinear(heightmap, width, height, x, z) * maxHeight;
            
            v.z = z;
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
    for (int j = 0; j < actualHeight - 1; j++) {
        for (int i = 0; i < actualWidth - 1; i++) {
            unsigned int topLeft = i + j * actualWidth;
            unsigned int topRight = (i + 1) + j * actualWidth;
            unsigned int bottomLeft = i + (j + 1) * actualWidth;
            unsigned int bottomRight = (i + 1) + (j + 1) * actualWidth;
            
            // Triangle 1
            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            
            // Triangle 2
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }
    
    // Calculer les normales
    calculateNormals(vertices, indices, normals);
    
    printf("Mesh generated: %zu vertices, %zu indices (resolution step: %.2f)\n", 
           vertices.size(), indices.size(), resolution);
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
