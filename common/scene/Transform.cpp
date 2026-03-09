#include "Transform.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

// IMPLÉMENTATION TEMPORAIRE - À COMPLÉTER PAR UN AUTRE DÉVELOPPEUR
// Cette implémentation basique devrait permettre de faire fonctionner le graphe de scène,
// mais doit être remplacée par une implémentation complète et optimisée

Transform::Transform()
    : position(0.0f, 0.0f, 0.0f)
    , rotation(1.0f, 0.0f, 0.0f, 0.0f)  // Quaternion identité
    , scale(1.0f, 1.0f, 1.0f)
    , parent(nullptr)
    , localMatrixDirty(true)
    , worldMatrixDirty(true)
{
    std::cerr << "ATTENTION: Transform.cpp utilise une implémentation temporaire !" << std::endl;
}

// === Position ===
void Transform::setPosition(const glm::vec3& pos) {
    position = pos;
    markDirty();
}

void Transform::translate(const glm::vec3& delta) {
    position += delta;
    markDirty();
}

glm::vec3 Transform::getPosition() const {
    return position;
}

glm::vec3 Transform::getWorldPosition() const {
    // TODO: À implémenter correctement
    glm::vec4 worldPos = getWorldMatrix() * glm::vec4(0, 0, 0, 1);
    return glm::vec3(worldPos);
}

// === Rotation ===
void Transform::setRotation(const glm::vec3& eulerAngles) {
    // TODO: À implémenter correctement
    rotation = glm::quat(eulerAngles);
    markDirty();
}

void Transform::setRotation(const glm::quat& quat) {
    rotation = quat;
    markDirty();
}

void Transform::rotate(const glm::vec3& axis, float angle) {
    // TODO: À implémenter correctement
    glm::quat rot = glm::angleAxis(angle, glm::normalize(axis));
    rotation = rot * rotation;
    markDirty();
}

void Transform::lookAt(const glm::vec3& target, const glm::vec3& up) {
    // TODO: À implémenter correctement
    std::cerr << "WARNING: Transform::lookAt() not implemented!" << std::endl;
}

glm::quat Transform::getRotation() const {
    return rotation;
}

glm::vec3 Transform::getEulerAngles() const {
    // TODO: À implémenter correctement
    return glm::eulerAngles(rotation);
}

// === Échelle ===
void Transform::setScale(const glm::vec3& s) {
    scale = s;
    markDirty();
}

void Transform::setUniformScale(float s) {
    scale = glm::vec3(s);
    markDirty();
}

glm::vec3 Transform::getScale() const {
    return scale;
}

// === Directions locales ===
glm::vec3 Transform::getForward() const {
    // TODO: À implémenter correctement
    return glm::normalize(rotation * glm::vec3(0, 0, -1));
}

glm::vec3 Transform::getRight() const {
    // TODO: À implémenter correctement
    return glm::normalize(rotation * glm::vec3(1, 0, 0));
}

glm::vec3 Transform::getUp() const {
    // TODO: À implémenter correctement
    return glm::normalize(rotation * glm::vec3(0, 1, 0));
}

// === Matrices ===
glm::mat4 Transform::getLocalMatrix() const {
    if (localMatrixDirty) {
        updateLocalMatrix();
    }
    return localMatrix;
}

glm::mat4 Transform::getWorldMatrix() const {
    if (worldMatrixDirty) {
        updateWorldMatrix();
    }
    return worldMatrix;
}

void Transform::updateLocalMatrix() const {
    // TODO: À implémenter correctement
    // Ordre typique : Translation * Rotation * Scale
    glm::mat4 T = glm::translate(glm::mat4(1.0f), position);
    glm::mat4 R = glm::mat4_cast(rotation);
    glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);
    
    localMatrix = T * R * S;
    localMatrixDirty = false;
}

void Transform::updateWorldMatrix() const {
    // TODO: À implémenter correctement
    if (parent) {
        worldMatrix = parent->getWorldMatrix() * getLocalMatrix();
    } else {
        worldMatrix = getLocalMatrix();
    }
    worldMatrixDirty = false;
}

// === Hiérarchie ===
void Transform::setParent(Transform* p) {
    parent = p;
    markDirty();
}

Transform* Transform::getParent() const {
    return parent;
}

void Transform::markDirty() {
    localMatrixDirty = true;
    worldMatrixDirty = true;
    // TODO: Propager le dirty flag aux enfants
}
