#ifndef MESHNODE_HPP
#define MESHNODE_HPP

#include "sceneNode.hpp"
#include "../render/mesh.hpp"

#include <GL/glew.h>

#include <memory>
#include <string>

/**
 * Nœud de scène avec géométrie (mesh)
 * Peut être rendu à l'écran
 */
class MeshNode : public SceneNode {
public:
    MeshNode(const std::string& name, std::shared_ptr<Mesh> mesh = nullptr);
    virtual ~MeshNode();
    
    // Surcharge du rendu pour dessiner le mesh
    virtual void draw(const glm::mat4& viewProjection) override;
    
    // Méthode virtuelle pour binder les textures
    virtual void bindTextures();
    
    // Accesseurs du mesh
    void setMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
    std::shared_ptr<Mesh> getMesh() const { return mesh; }

    void setShaderProgram(GLuint shaderProgram) { this->shaderProgram = shaderProgram; }
    GLuint getShaderProgram() const { return shaderProgram; }

    void setTexture(GLuint texture) { this->texture = texture; }
    GLuint getTexture() const { return texture; }
    
    /**
     * Charge un mesh depuis un fichier OFF (optimisé)
     * @param filename Chemin du fichier OFF
     * @param shaderProgram ID du program OpenGL à utiliser
     * @param nodeName Nom du nœud
     * @param enableCache Si true, le mesh sera mis en cache pour éviter les rechargements
     * @return Pointeur partagé vers le MeshNode créé, nullptr en cas d'erreur
     */
    static std::shared_ptr<MeshNode> loadFromOFF(
        const std::string& filename,
        GLuint shaderProgram,
        const std::string& nodeName = "Mesh",
        bool enableCache = true
    );
    
    /**
     * Vide le cache des meshes chargés
     * À appeler lors de la fermeture de l'application
     */
    static void clearMeshCache();
    
protected:
    std::shared_ptr<Mesh> mesh;
    GLuint shaderProgram;
    GLuint texture;
};

#endif
