#include "meshNode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <iostream>

MeshNode::MeshNode(const std::string& name, std::shared_ptr<Mesh> mesh)
    : SceneNode(name)
    , mesh(mesh)
    , shaderProgram(0)
    , texture(0)
{}

MeshNode::~MeshNode() {
    // Le mesh est géré par shared_ptr, donc libération automatique
}

void MeshNode::bindTextures() {
    // Binder la texture unique (mesh simple)
    if (texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        GLuint textureID = glGetUniformLocation(shaderProgram, "textureSampler");
        if (textureID != (GLuint)-1) {
            glUniform1i(textureID, 0);
        }
    }
}

void MeshNode::draw(const glm::mat4& viewProjection) {
    if (!isActive || !mesh || mesh->indexCount == 0) {
        // Dessiner quand même les enfants
        SceneNode::draw(viewProjection);
        return;
    }

    if (!mesh->hasGPUData()) {
        mesh->uploadToGPU();
    }

    if (shaderProgram == 0) {
        SceneNode::draw(viewProjection);
        return;
    }
    
    // Utiliser le shader du mesh
    glUseProgram(shaderProgram);
    
    // Calculer la matrice MVP
    glm::mat4 model = transform->getWorldMatrix();
    glm::mat4 MVP = viewProjection * model;
    
    // Envoyer la matrice MVP au shader
    GLuint mvpID = glGetUniformLocation(shaderProgram, "MVP");
    if (mvpID != (GLuint)-1) {
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);
    }
    
    // Envoyer la matrice Model au shader (pour l'éclairage)
    GLuint modelID = glGetUniformLocation(shaderProgram, "M");
    if (modelID != (GLuint)-1) {
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
    }
    
    bindTextures();
    
    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // UVs (si présents)
    if (mesh->uvBuffer != 0) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Normals (si présents)
    if (mesh->normalBuffer != 0) {
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->normalBuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->indexBuffer);
    
    // Dessiner
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, (void*)0);
    
    // Désactiver les attributs
    glDisableVertexAttribArray(0);
    if (mesh->uvBuffer != 0) glDisableVertexAttribArray(1);
    if (mesh->normalBuffer != 0) glDisableVertexAttribArray(2);
    
    // Dessiner les enfants
    SceneNode::draw(viewProjection);
}

// === Chargement optimisé depuis fichier OFF ===
std::shared_ptr<MeshNode> MeshNode::loadFromOFF(
    const std::string& filename,
    GLuint shaderProgram,
    const std::string& nodeName,
    bool enableCache) {
    auto mesh = Mesh::loadFromOFF(filename, enableCache);
    if (!mesh) {
        return nullptr;
    }

    if (mesh->normals.empty()) {
        mesh->computeNormals();
    }
    if (mesh->uvs.empty()) {
        mesh->computeUVs();
    }

    mesh->uploadToGPU();

    auto node = std::make_shared<MeshNode>(nodeName, mesh);
    node->setShaderProgram(shaderProgram);
    return node;
}

void MeshNode::clearMeshCache()
{
    Mesh::clearMeshCache();
}
