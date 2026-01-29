#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <vector>
#include <string>
#include <glm/glm.hpp>

// Structure pour les paramètres de vue de caméra
struct CameraSetup {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
    float speed;  // Vitesse de déplacement suggérée
};

class Terrain {
public:
    Terrain();
    
    // Chargement et génération du terrain
    bool loadHeightmap(const std::string& filepath, float maxHeight = 50.0f);
    void generateFlatPlane(int width, int height);
    void generateMesh(std::vector<glm::vec3>& vertices, 
                      std::vector<unsigned int>& indices,
                      std::vector<glm::vec2>& uvs,
                      std::vector<glm::vec3>& normals);
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    glm::vec3 getCenterPosition() const;
    
    // Calcul de vue optimale pour la caméra
    CameraSetup getOptimalIsometricView() const;
    
private:
    int width;
    int height;
    float maxHeight;
    std::vector<std::vector<float>> heightmap;
    
    // Méthode privée pour calculer les normales
    void calculateNormals(const std::vector<glm::vec3>& vertices,
                          const std::vector<unsigned int>& indices,
                          std::vector<glm::vec3>& normals);
};

#endif
