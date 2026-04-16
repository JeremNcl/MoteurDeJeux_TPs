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
    // === CONSTRUCTEUR ===
    Terrain();
    
    // === GENERATION DE TERRAIN ===
    bool loadHeightmap(const std::string& filepath, float maxHeight = 50.0f);
    void generateFlatPlane(int width, int height);
    void generateMesh(Mesh& mesh);
    
    // === GESTION DE LA RÉSOLUTION (LOD) ===
    void setResolution(float step);  // step = 0.5 (détail maximum), 1, 2, 4, etc.
    float getResolution() const { return resolution; }
    float getMinResolution() const { return 0.25f; }  // Résolution minimale (1/4)
    float getMaxResolution() const { return 1.0f; }   // Résolution maximale
    
    // === GETTERS ===
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getMaxHeight() const { return maxHeight; }
    glm::vec3 getCenterPosition() const;
    std::vector<std::vector<float>> getHeightmap() const { return heightmap; }
    
    // === CALCUL DE VUE CAMÉRA ===
    CameraSetup getOptimalIsometricView() const;

private:
    // === ATTRIBUTS ===
    int width;
    int height;
    float maxHeight;
    float resolution; 
    std::vector<std::vector<float>> heightmap;
};

#endif
