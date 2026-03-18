#include "transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

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

glm::vec3 Transform::getWorldScale() const
{
    if (parent == nullptr) return s;

    return parent->getWorldScale() * s;
}

Transform::~Transform()
{
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

//TODO : C'est une rotation dégueux pour tester, passer après par euler où quat
glm::mat3 Transform::getWorldRotation() const
{
    if (parent == nullptr) return r;

    return parent->getWorldRotation() * r;
}

glm::vec3 Transform::getWorldTranslation() const
{
   /*  if (parent == nullptr) return t;

    return parent->getWorldTranslation() + t; */
    if (parent == nullptr) return t;

    // 1. On récupère la position, rotation et scale du parent dans le monde
    glm::vec3 parentScale = parent->getWorldScale();
    glm::mat3 parentRot   = parent->getWorldRotation();
    glm::vec3 parentPos   = parent->getWorldTranslation();

    // 2. On ajuste la translation locale de l'enfant en fonction du parent
    // On applique d'abord le scale, puis la rotation du parent à notre translation locale
    glm::vec3 combinedTranslation = parentRot * (parentScale * t);

    // 3. On ajoute le tout à la position du parent
    return parentPos + combinedTranslation;
}

void Transform::setParent(Transform* _parent)
{
    if (parent != nullptr) parent->removeChild(this);

    parent = _parent;

    if (parent != nullptr) parent-> addChild(this);

    //SI PROBLEME AU CHANGEMENT DE PARENT
    /* // 1. Éviter l'auto-parentage ou le parentage redondant
    if (_newParent == this || _newParent == parent) return;

    // 2. Maintenir la position mondiale (World Persistence)
    // On récupère nos transformations actuelles dans le monde
    glm::vec3 worldPos = getWorldTranslation();
    glm::mat3 worldRot = getWorldRotation();
    glm::vec3 worldScale = getWorldScale();

    // 3. Détachement de l'ancien parent
    if (parent != nullptr) {
        // Suppression manuelle pour éviter les boucles infinies
        auto& children = parent->children;
        children.erase(std::remove(children.begin(), children.end(), this), children.end());
    }

    // 4. Attribution du nouveau parent
    parent = _newParent;

    if (parent != nullptr) {
        // On ajoute cet objet aux enfants du nouveau parent
        parent->children.push_back(this);

        // On recalcule nos propriétés LOCALES pour que, multipliées par celles 
        // du nouveau parent, elles redonnent nos anciennes propriétés MONDIALES.
        
        // Inverse de la rotation du parent * notre rotation mondiale
        r = glm::inverse(parent->getWorldRotation()) * worldRot;
        
        // Notre translation mondiale relative au repère du parent
        // On soustrait la position du parent et on annule sa rotation/scale
        glm::vec3 relativePos = worldPos - parent->getWorldTranslation();
        t = glm::inverse(parent->getWorldRotation()) * (relativePos / parent->getWorldScale());
        
        // Scale relatif
        s = worldScale / parent->getWorldScale();
    } else {
        // Si on devient une racine (parent nul), le local devient le mondial
        t = worldPos;
        r = worldRot;
        s = worldScale;
    } */
}

void Transform::addChild(Transform* _child)
{
    if (_child == nullptr || _child == this) return;

    for (Transform* c : children)
    {
      if (c == _child) return;
    }

    children.push_back(_child);

    if (_child->parent != this) _child->parent = this;
}

void Transform::removeChild(Transform* _child){

    auto it = std::remove(children.begin(), children.end(), _child);
    
    if (it != children.end()) {
        children.erase(it, children.end());
        _child->parent = nullptr;
    }

}

void Transform::removeAllChildren()
{
    for (auto child : children) {
        if (child != nullptr) {
            child->parent = nullptr;
        }
    }
    children.clear();
}

void Transform::scale(const glm::vec3& _s)
{
    s *= _s;
}

void Transform::rotate(const glm::mat3& _r)
{
    r = _r * r;
}

void Transform::translate(const glm::vec3& _t)
{
    t += _t;
}

glm::mat4 Transform::getWorldMatrix() const
{
    // 1. Création de la matrice locale (4x4)
    // On part d'une matrice identité
    glm::mat4 localMatrix = glm::mat4(1.0f);

    // Appliquer la Translation
    localMatrix = glm::translate(localMatrix, t);

    // Appliquer la Rotation (Conversion de mat3 à mat4)
    localMatrix = localMatrix * glm::mat4(r);

    // Appliquer le Scale
    localMatrix = glm::scale(localMatrix, s);

    // 2. Récursion : si on a un parent, on multiplie par sa World Matrix
    if (parent != nullptr) {
        return parent->getWorldMatrix() * localMatrix;
    }

    return localMatrix;
}