#include "transform.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <algorithm>
#include <memory>


// === Constructeurs et destructeur ===
Transform::Transform(std::weak_ptr<Transform> _parent)
    : s(glm::vec3(1.0f)),
      r(glm::mat3(1.0f)),
      t(glm::vec3(0.0f)),
      parent(_parent)
{}

Transform::Transform(const glm::vec3& _s, const glm::mat3& _r, const glm::vec3& _t, std::weak_ptr<Transform> _parent)
    : s(_s),
      r(_r),
      t(_t),
      parent(_parent)
{}

Transform::Transform(const Transform& _transform, std::weak_ptr<Transform> _parent)
    : s(_transform.s),
      r(_transform.r),
      t(_transform.t),
      parent(_parent)
{}

Transform::~Transform() {
    auto p = parent.lock();
    if(p) {
        auto& pChildren = p->children;
        pChildren.erase(std::remove_if(pChildren.begin(), pChildren.end(), [this](const std::shared_ptr<Transform>& c){ return c.get() == this; }), pChildren.end());
    }
    for (auto& child : children) {
        if (child) {
            child->parent.reset();
        }
    }
    children.clear();
}

// === Getters ===
glm::vec3 Transform::getWorldScale() const {
    auto p = parent.lock();
    if (!p) return s;

    return p->getWorldScale() * s;
}

glm::mat3 Transform::getWorldRotation() const {
    auto p = parent.lock();
    if (!p) return r;

    return p->getWorldRotation() * r;
}

glm::vec3 Transform::getWorldTranslation() const {
    auto p = parent.lock();
    if (!p) return t;

    glm::vec3 parentScale = p->getWorldScale();
    glm::mat3 parentRot   = p->getWorldRotation();
    glm::vec3 parentPos   = p->getWorldTranslation();
    glm::vec3 combinedTranslation = parentRot * (parentScale * t);

    return parentPos + combinedTranslation;
}

glm::mat4 Transform::getWorldMatrix() const {
    if (!worldMatrixDirty) return cachedWorldMatrix;
    
    // Création de la matrice locale (4x4)
    glm::mat4 localMatrix = glm::mat4(1.0f);
    localMatrix = glm::translate(localMatrix, t);
    localMatrix = localMatrix * glm::mat4(r);
    localMatrix = glm::scale(localMatrix, s);

    auto p = parent.lock();
    if (p) {
        cachedWorldMatrix = p->getWorldMatrix() * localMatrix;
    } else {
        cachedWorldMatrix = localMatrix;
    }

    worldMatrixDirty = false;
    return cachedWorldMatrix;
}

// === Gestion hiérarchie ===
void Transform::addChild(const std::shared_ptr<Transform>& _child)
{
    if (!_child || _child.get() == this) return;
    if (isAncestorOf(_child.get())) return;
    for (auto& c : children) {
        if (c == _child) return;
    }
    children.push_back(_child);
    if (_child->parent.lock().get() != this) _child->setParent(shared_from_this());
    markWorldMatrixDirty();
}

void Transform::removeChild(const std::shared_ptr<Transform>& _child){
    auto it = std::remove(children.begin(), children.end(), _child);
    if (it != children.end()) {
        children.erase(it, children.end());
        if (_child->parent.lock().get() == this) _child->parent.reset();
    }
    markWorldMatrixDirty();
}

void Transform::removeAllChildren() {
    for (auto& child : children) {
        if (child && child->parent.lock().get() == this) {
            child->parent.reset();
        }
    }
    children.clear();
    markWorldMatrixDirty();
}

void Transform::setParent(const std::shared_ptr<Transform>& _parent)
{
    if (_parent.get() == this || (_parent && _parent->isAncestorOf(this))) return;
    auto p = parent.lock();
    if (p) p->removeChild(shared_from_this());
    parent = _parent;
    if (_parent) _parent->addChild(shared_from_this());
    markWorldMatrixDirty();
}

void Transform::markWorldMatrixDirty() {
    worldMatrixDirty = true;
    for (auto& child : children) {
        if (child) child->markWorldMatrixDirty();
    }
}

// === Setters ===
void Transform::setScale(const glm::vec3& _s) { 
    s = _s;
    markWorldMatrixDirty();
}

void Transform::setRotation(const glm::mat3& _r) {
    r = _r;
    markWorldMatrixDirty();
}

void Transform::setRotation(const glm::quat& q) {
    r = glm::mat3_cast(q);
    markWorldMatrixDirty();
}

void Transform::setRotation(const glm::vec3& eulerAngles) {
    r = glm::mat3(glm::yawPitchRoll(eulerAngles.x, eulerAngles.y, eulerAngles.z));
    markWorldMatrixDirty();
}

void Transform::setTranslation(const glm::vec3& _t) {
    t = _t; markWorldMatrixDirty();
}

// === Utilitaires ===
bool Transform::isAncestorOf(const Transform* _other) const {
    if (!_other) return false;
    auto current = _other->parent.lock();
    while (current) {
        if (current.get() == this) return true;
        current = current->parent.lock();
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

void Transform::rotate(const glm::quat& q) {
    r = glm::mat3_cast(q) * r;
    markWorldMatrixDirty();
}

void Transform::rotate(const glm::vec3& eulerAngles) {
    glm::mat3 rot = glm::mat3(glm::yawPitchRoll(eulerAngles.x, eulerAngles.y, eulerAngles.z));
    r = rot * r;
    markWorldMatrixDirty();
}

void Transform::translate(const glm::vec3& _t) {
    t += _t;
    markWorldMatrixDirty();
}