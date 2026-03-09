#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * Classe Transform : encapsule les transformations spatiales d'un objet 3D
 * Gère la position, rotation, échelle et les matrices de transformation
 * À IMPLÉMENTER par un autre développeur
 */
class Transform {
public:
    Transform();
    
    // === Position ===
    void setPosition(const glm::vec3& pos);
    void translate(const glm::vec3& delta);
    glm::vec3 getPosition() const;
    glm::vec3 getWorldPosition() const;
    
    // === Rotation ===
    void setRotation(const glm::vec3& eulerAngles); // en radians (pitch, yaw, roll)
    void setRotation(const glm::quat& quat);
    void rotate(const glm::vec3& axis, float angle);
    void lookAt(const glm::vec3& target, const glm::vec3& up = glm::vec3(0, 1, 0));
    glm::quat getRotation() const;
    glm::vec3 getEulerAngles() const;
    
    // === Échelle ===
    void setScale(const glm::vec3& scale);
    void setUniformScale(float scale);
    glm::vec3 getScale() const;
    
    // === Directions locales (utile pour naviguer dans l'espace) ===
    glm::vec3 getForward() const;  // Direction "devant" (axe -Z en OpenGL)
    glm::vec3 getRight() const;    // Direction "droite" (axe +X)
    glm::vec3 getUp() const;       // Direction "haut" (axe +Y)
    
    // === Matrices de transformation ===
    glm::mat4 getLocalMatrix() const;  // Matrice de transformation locale
    glm::mat4 getWorldMatrix() const;  // Matrice de transformation dans l'espace monde
    
    // === Hiérarchie ===
    void setParent(Transform* parent);
    Transform* getParent() const;
    
    // Marquer les matrices comme "dirty" (à recalculer)
    void markDirty();
    
private:
    // Transformations locales
    glm::vec3 position;
    glm::quat rotation;  // Quaternion pour éviter le gimbal lock
    glm::vec3 scale;
    
    // Hiérarchie
    Transform* parent;
    
    // Cache des matrices
    mutable glm::mat4 localMatrix;
    mutable glm::mat4 worldMatrix;
    mutable bool localMatrixDirty;
    mutable bool worldMatrixDirty;
    
    // Méthodes privées pour recalculer les matrices
    void updateLocalMatrix() const;
    void updateWorldMatrix() const;
};

#endif
