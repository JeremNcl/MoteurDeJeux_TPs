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
    TerrainNode(const std::string& name, Terrain& terrain, std::shared_ptr<Mesh> mesh = nullptr);
    virtual ~TerrainNode();
    
    // Surcharge du rendu pour gérer le multi-texturing
    virtual void draw(const glm::mat4& viewProjection) override;

    // Méthode virtuelle pour binder les textures
    virtual void bindTextures() override;
    
    // Configuration des textures du terrain
    void setTextures(GLuint textureLow, GLuint textureMid, GLuint textureHigh);
    void setHeightParameters(float heightLow, float heightMid, float blendRange);
    
    // Accès au terrain pour modification
    Terrain& getTerrain() { return terrain; }
    const Terrain& getTerrain() const { return terrain; }
    
    // Régénération du mesh si le terrain change
    void regenerateMesh();
    
private:
    Terrain& terrain;
    
    // Textures multi-couches
    GLuint texture_low;
    GLuint texture_mid;
    GLuint texture_high;
    
    // Paramètres de blending des textures
    float height_low;
    float height_mid;
    float blend_range;
};

#endif
