#include "transform.hpp"
#include <glm/glm.hpp>
#include <algorithm>

Transform::Transform(Transform* _parent)
    : parent(_parent)
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
    if (parent == nullptr) parent->removeChild(this);

    parent = _parent;

    if (parent != nullptr) parent-> addChild(this);
}

void Transform::addChild(Transform* _child)
{
    if (_child == nullptr || _child == this) return;

    for (Transform c : children)
    {
      if (c == _child) return;
    }

    children.push_back(_child);

    if (_child->parent != this) _child->parent = this;
}

void Transform::removeChild(Transform* _child){

    auto it = std::remove(children.begin(), children.end(), child);
    
    if (it != children.end()) {
        children.erase(it, children.end());
        child->parent = nullptr;
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