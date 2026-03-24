#ifndef TERRAINNODE_HPP
#define TERRAINNODE_HPP

#include "../scene/meshNode.hpp"
#include "terrain.hpp"


/**
 * Nœud spécialisé pour le rendu du terrain
 * Gère le multi-texturing et les paramètres spécifiques au terrain
 */
class TerrainNode : public MeshNode {
public:
    // === CONSTRUCTEUR ===
    TerrainNode(const std::string& name, Terrain& terrain, std::shared_ptr<Mesh> mesh = nullptr);
    
    // === DESTRUCTEUR ===
    virtual ~TerrainNode();

    // === SETTERS ===
    void setTextures(GLuint textureLow, GLuint textureMid, GLuint textureHigh);
    void setHeightParameters(float heightLow, float heightMid, float blendRange);
    
    // === GETTERS ===
    Terrain& getTerrain() { return terrain; }
    const Terrain& getTerrain() const { return terrain; }

    // === RENDU ===
    virtual void draw(const glm::mat4& viewProjection) override;
    
    // === GESTION DU MESH ===
    void regenerateMesh();

private:
    // === TERRAIN ===
    Terrain& terrain;
    
    // === TEXTURES ===
    GLuint texture_low;
    GLuint texture_mid;
    GLuint texture_high;
    // Paramètres de blending des textures
    float height_low;
    float height_mid;
    float blend_range;

    // === GESTION DES TEXTURES ===
    virtual void bindTextures() override;
};

#endif
