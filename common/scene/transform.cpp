#include "transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


// === Constructeurs et destructeur ===
Transform::Transform(Transform* _parent)
    : s(glm::vec3(1.0f)),
      r(glm::mat3(1.0f)),
      t(glm::vec3(0.0f)),
      parent(_parent)
{}

Transform::Transform(const glm::vec3& _s, const glm::mat3& _r, const glm::vec3& _t, Transform* _parent)
    : s(_s),
      r(_r),
      t(_t),
      parent(_parent)
{}

Transform::Transform(const Transform& _transform, Transform* _parent)
    : s(_transform.s),
      r(_transform.r),
      t(_transform.t),
      parent(_parent)
{}

Transform::~Transform() {
    if(parent != nullptr)
    {
        auto& pChildren = parent->children;
        pChildren.erase(std::remove(pChildren.begin(), pChildren.end(), this), pChildren.end());
    }

    for (Transform* child : children) {
        if (child != nullptr) {
            child->parent = nullptr;
        }
    }

    children.clear();
}

// === Getters ===
glm::vec3 Transform::getWorldScale() const {
    if (parent == nullptr) return s;
    return parent->getWorldScale() * s;
}

glm::mat3 Transform::getWorldRotation() const {
    if (parent == nullptr) return r;
    return parent->getWorldRotation() * r;
}

glm::vec3 Transform::getWorldTranslation() const {
    if (parent == nullptr) return t;
    
    // On récupère la position, rotation et scale du parent dans le monde
    glm::vec3 parentScale = parent->getWorldScale();
    glm::mat3 parentRot   = parent->getWorldRotation();
    glm::vec3 parentPos   = parent->getWorldTranslation();
    
    // On ajuste la translation locale de l'enfant en fonction du parent
    // On applique d'abord le scale, puis la rotation du parent à notre translation locale
    glm::vec3 combinedTranslation = parentRot * (parentScale * t);
    
    // On ajoute le tout à la position du parent
    return parentPos + combinedTranslation;
}

glm::mat4 Transform::getWorldMatrix() const {
    if (!worldMatrixDirty) return cachedWorldMatrix;
    
    // Création de la matrice locale (4x4)
    glm::mat4 localMatrix = glm::mat4(1.0f);
    localMatrix = glm::translate(localMatrix, t);
    localMatrix = localMatrix * glm::mat4(r);
    localMatrix = glm::scale(localMatrix, s);
    
    // Récursion : si on a un parent, on multiplie par sa World Matrix
    if (parent != nullptr) {
        cachedWorldMatrix = parent->getWorldMatrix() * localMatrix;
    } else {
        cachedWorldMatrix = localMatrix;
    }

    worldMatrixDirty = false;
    return cachedWorldMatrix;
}

// === Gestion hiérarchie ===
void Transform::addChild(Transform* _child)
{
    if (_child == nullptr || _child == this) return;
    
    // Protection contre les cycles : ne pas ajouter si _child est un ancêtre
    if (isAncestorOf(_child)) return;

    for (Transform* c : children) {
      if (c == _child) return;
    }

    children.push_back(_child);

    // Symétrie parent/enfant
    if (_child->parent != this) _child->setParent(this);

    markWorldMatrixDirty();
}

void Transform::removeChild(Transform* _child){
    auto it = std::remove(children.begin(), children.end(), _child);
    if (it != children.end()) {
        children.erase(it, children.end());
        // Symétrie parent/enfant
        if (_child->parent == this) _child->parent = nullptr;
    }

    markWorldMatrixDirty();
}

void Transform::removeAllChildren() {
    for (auto child : children) {
        if (child != nullptr && child->parent == this) {
            child->parent = nullptr;
        }
    }
    
    children.clear();

    markWorldMatrixDirty();
}

void Transform::setParent(Transform* _parent) {
    // Protection contre les cycles : ne pas ajouter si _parent est un descendant
    if (_parent == this || (_parent && _parent->isAncestorOf(this))) return;

    if (parent != nullptr) parent->removeChild(this);
    parent = _parent;

    if (parent != nullptr) parent->addChild(this);

    markWorldMatrixDirty();
}

void Transform::markWorldMatrixDirty() {
    worldMatrixDirty = true;
    for (Transform* child : children) {
        if (child) child->markWorldMatrixDirty();
    }
}

// === Utilitaires ===
bool Transform::isAncestorOf(const Transform* _other) const {
    if (_other == nullptr) return false;
    const Transform* current = _other->parent;
    while (current != nullptr) {
        if (current == this) return true;
        current = current->parent;
    }
    return false;
}

// === Transformations locales ===
void Transform::scale(const glm::vec3& _s) {
    s *= _s;
    markWorldMatrixDirty();
}

void Transform::rotate(const glm::mat3& _r) {
    r = _r * r;
    markWorldMatrixDirty();
}

void Transform::translate(const glm::vec3& _t) {
    t += _t;
    markWorldMatrixDirty();
}