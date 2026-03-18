#include "SceneNode.hpp"
#include <algorithm>

SceneNode::SceneNode(const std::string& name)
    : name(name)
    , isActive(true)
    , parent(nullptr)
{}

SceneNode::~SceneNode() {
    // Détacher tous les enfants
    for (auto& child : children) {
        if (child) {
            child->parent = nullptr;
        }
    }
}

void SceneNode::addChild(std::shared_ptr<SceneNode> child) {
    if (!child) return;
    
    // Retirer l'enfant de son ancien parent
    if (child->parent) {
        child->parent->removeChild(child);
    }
    
    // Ajouter comme enfant
    children.push_back(child);
    child->parent = this;
    
    // Mettre à jour la hiérarchie des transforms
    child->transform.setParent(&this->transform);
}

void SceneNode::removeChild(std::shared_ptr<SceneNode> child) {
    if (!child) return;
    
    auto it = std::find(children.begin(), children.end(), child);
    if (it != children.end()) {
        (*it)->parent = nullptr;
        (*it)->transform.setParent(nullptr);
        
        children.erase(it);
    }
}

void SceneNode::update(float deltaTime) {
    if (!isActive) return;
    
    // Mettre à jour tous les enfants récursivement
    for (auto& child : children) {
        if (child && child->getActive()) {
            child->update(deltaTime);
        }
    }
}

void SceneNode::draw(const glm::mat4& viewProjection) {
    if (!isActive) return;
    
    // Dessiner tous les enfants récursivement
    for (auto& child : children) {
        if (child && child->getActive()) {
            child->draw(viewProjection);
        }
    }
}
