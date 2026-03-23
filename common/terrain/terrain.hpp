#ifndef TERRAIN_HPP
#define TERRAIN_HPP

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "../render/mesh.hpp"


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
    void generateMesh(Mesh& mesh);
    
    // Gestion de la résolution (LOD)
    void setResolution(float step);  // step = 0.5 (détail maximum), 1, 2, 4, etc.
    float getResolution() const { return resolution; }
    float getMinResolution() const { return 0.25f; }  // Résolution minimale (1/4)
    float getMaxResolution() const { return 1.0f; }   // Résolution maximale
    
    // Getters
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    glm::vec3 getCenterPosition() const;
    
    // Calcul de vue optimale pour la caméra
    CameraSetup getOptimalIsometricView() const;

    std::vector<std::vector<float>> heightmap;
    
private:
    int width;
    int height;
    float maxHeight;
    float resolution;  // Pas de sous-échantillonnage (0.5 = sur-échantillonnage x2, 1 = normal, 2 = /2, 4 = /4, etc.)
    
    
    // Méthode privée pour calculer les normales
    void calculateNormals(const std::vector<glm::vec3>& vertices,
                          const std::vector<unsigned int>& indices,
                          std::vector<glm::vec3>& normals);
};

#endif
