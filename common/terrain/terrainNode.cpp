#include "terrainNode.hpp"
#include <iostream>

TerrainNode::TerrainNode(const std::string& name, Terrain& terrain, std::shared_ptr<Mesh> mesh)
    : MeshNode(name, mesh)
    , terrain(terrain)
    , texture_low(0)
    , texture_mid(0)
    , texture_high(0)
    , height_low(40.0f)
    , height_mid(80.0f)
    , blend_range(10.0f)
{}

TerrainNode::~TerrainNode() {
    // Les textures sont gérées ailleurs (dans main)
}

void TerrainNode::setTextures(GLuint textureLow, GLuint textureMid, GLuint textureHigh) {
    texture_low = textureLow;
    texture_mid = textureMid;
    texture_high = textureHigh;
}

void TerrainNode::setHeightParameters(float heightLow, float heightMid, float blendRange) {
    this->height_low = heightLow;
    this->height_mid = heightMid;
    this->blend_range = blendRange;
}

void TerrainNode::bindTextures() {
    // Binder les textures multi-couches terrain
    if (texture_low != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_low);
        GLuint textureLowID = glGetUniformLocation(mesh->shaderProgram, "texture_low");
        if (textureLowID != (GLuint)-1) {
            glUniform1i(textureLowID, 0);
        }
    }
    if (texture_mid != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture_mid);
        GLuint textureMidID = glGetUniformLocation(mesh->shaderProgram, "texture_mid");
        if (textureMidID != (GLuint)-1) {
            glUniform1i(textureMidID, 1);
        }
    }
    if (texture_high != 0) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texture_high);
        GLuint textureHighID = glGetUniformLocation(mesh->shaderProgram, "texture_high");
        if (textureHighID != (GLuint)-1) {
            glUniform1i(textureHighID, 2);
        }
    }
}

void TerrainNode::draw(const glm::mat4& viewProjection) {
    if (!isActive || !mesh || mesh->indexCount == 0) {
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
    glUseProgram(mesh->shaderProgram);
    
    // Calculer les matrices
    glm::mat4 model = transform->getWorldMatrix();
    glm::mat4 MVP = viewProjection * model;
    
    GLuint mvpID = glGetUniformLocation(mesh->shaderProgram, "MVP");
    if (mvpID != (GLuint)-1) {
        glUniformMatrix4fv(mvpID, 1, GL_FALSE, &MVP[0][0]);
    }
    
    // Envoyer la matrice Model (pour l'éclairage)
    GLuint modelID = glGetUniformLocation(mesh->shaderProgram, "M");
    if (modelID != (GLuint)-1) {
        glUniformMatrix4fv(modelID, 1, GL_FALSE, &model[0][0]);
    }
    
    // Binder les textures multi-couches terrain
    bindTextures();
    
    // Paramètres de hauteur pour le blending
    GLuint heightLowID = glGetUniformLocation(mesh->shaderProgram, "height_low");
    if (heightLowID != (GLuint)-1) {
        glUniform1f(heightLowID, height_low);
    }
    
    GLuint heightMidID = glGetUniformLocation(mesh->shaderProgram, "height_mid");
    if (heightMidID != (GLuint)-1) {
        glUniform1f(heightMidID, height_mid);
    }
    
    GLuint blendRangeID = glGetUniformLocation(mesh->shaderProgram, "blend_range");
    if (blendRangeID != (GLuint)-1) {
        glUniform1f(blendRangeID, blend_range);
    }
    
    // Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexBuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    // UVs
    if (mesh->uvBuffer != 0) {
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->uvBuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    }
    
    // Normals
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

void TerrainNode::regenerateMesh() {
    if (!mesh) return;
    
    terrain.generateMesh(*mesh);
}
