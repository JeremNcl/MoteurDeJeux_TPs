#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Mesh {
public:
    // === ATTRIBUTS ===
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    // === CONSTRUCTEUR ===
    Mesh();
    static std::shared_ptr<Mesh> generateSphere(float radius, int meridianCount, int parallelCount);
    // Chargement depuis fichier OFF (avec cache optionnel)
    static std::shared_ptr<Mesh> loadFromOFF(const std::string& filename, bool enableCache = true);
    
    // === DESTRUCTEUR ===
    ~Mesh();

    // === CALCUL DES NORMALES ET UVs ===
    void computeNormals();
    void computeUVs();

    // === GESTION DU CACHE DE MESHES ===
    static void clearMeshCache();

private:
    // === PARAMETRES SPHERIQUES ===
    int meridianCount = 0;
    int parallelCount = 0;
    // Getters
    int getMeridianCount() const { return meridianCount; }
    int getParallelCount() const { return parallelCount; }
    // Setters
    void setMeridianCount(int m) { meridianCount = m; }
    void setParallelCount(int p) { parallelCount = p; }

    // === CACHE DE MESHES ===
    static std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache;
};

#endif
