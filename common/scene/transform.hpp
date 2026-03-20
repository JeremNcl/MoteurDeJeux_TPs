#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include <vector>
#include <memory>


class Transform : public std::enable_shared_from_this<Transform> {

    // === Membres privés : état interne et hiérarchie ===
private:
    glm::vec3 s{1.f}; // uniform scale
    glm::mat3 r{0.f}; // rotation matrix
    glm::vec3 t{0.f}; // translation vector

    std::weak_ptr<Transform> parent;
    std::vector<std::shared_ptr<Transform>> children;
    
    mutable glm::mat4 cachedWorldMatrix{1.0f};
    mutable bool worldMatrixDirty{true};

    // === API publique : interface, gestion hiérarchie, transformations ===
public: 
    // === Constructeurs et destructeur ===
    Transform(std::weak_ptr<Transform> parent = {});
    Transform(const glm::vec3& _s, const glm::mat3& _r, const glm::vec3& _t, std::weak_ptr<Transform> _parent = {});
    Transform(const Transform& _transform, std::weak_ptr<Transform> _parent = {});
    ~Transform();

    // === Getters ===
    glm::vec3 getLocalScale() const { return s; }   
    glm::mat3 getLocalRotation() const { return r; }
    glm::vec3 getLocalTranslation() const { return t; }

    glm::vec3 getWorldScale() const;
    glm::mat3 getWorldRotation() const;
    glm::vec3 getWorldTranslation() const;

    glm::mat4 getWorldMatrix() const;

    std::shared_ptr<Transform> getParent() const { return parent.lock(); }
    std::vector<std::shared_ptr<Transform>> getAllChildren() const { return children; }

    // === Setters ===
    void setScale(const glm::vec3& _s);
    void setRotation(const glm::mat3& _r);
    void setRotation(const glm::quat& q);
    void setRotation(const glm::vec3& eulerAngles);
    void setTranslation(const glm::vec3& _t);
    
    // === Gestion hiérarchie ===
    void addChild(const std::shared_ptr<Transform>& _child);
    void removeChild(const std::shared_ptr<Transform>& _child);
    void removeAllChildren();
    void setParent(const std::shared_ptr<Transform>& _parent);

    void markWorldMatrixDirty();

    // === Utilitaires ===
    bool isAncestorOf(const Transform* _other) const;

    // === Transformations locales ===
    void scale(const glm::vec3& _s);
    void rotate(const glm::mat3& _r);
    void rotate(const glm::quat& q); // surcharge quaternion
    void rotate(const glm::vec3& eulerAngles); // surcharge angles d'Euler (radians)
    void translate(const glm::vec3& _t);   
};

#endif