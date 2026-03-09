#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <vector>

class Transform {

    public: 

        Transform(Transform* parent = nullptr);
        Transform(const glm::vec3& _s, const glm::vec3& _r, const glm::vec3& _t, Transform* _parent = nullptr);
        Transform(const Transform& _transform, Transform* _parent = nullptr);

        //Getter

        glm::vec3 getScale() const { return s; }
        glm::vec3 getRotation() const { return r; }
        glm::vec3 getTranslation() const { return t; }

        glm::vec3 getLocalScale() const;
        glm::vec3 getLocalRotation() const;
        glm::vec3 getLocalTranslation() const;

        const Transform* getParent() const { return parent; }
        Transform* getParent() { return parent; }
    
        const std::vector<Transform*> getAllChildrens() const { return childrens; }
        std::vector<Transform*> getAllChildrens() { return childrens; }
        
        //Setter
        
        void setScale(const glm::vec3& _s) { s = _s; }
        void setRotation(const glm::vec3& _r) { r = _r; }
        void setTranslation(const glm::vec3& _t) { t = _t; }

        void setParent(Transform* _parent) { parent = _parent; }

        //Other

        void addChildren(Transform* _children);
        void removeChildren(Transform* _children);
        void removeAllChildren();

        //Modifier 

        void scale(const glm::vec3& _s);
        void rotate(const glm::vec3& _r);
        void translate(const glm::vec3& _t);

        void rotateAround(const Transform _transform);

    private:
        
        glm::vec3 s{1.f}; // uniform scale
        glm::vec3 r{0.f}; // rotation matrix
        glm::vec3 t{0.f}; // translation vector

        Transform* parent;
        std::vector<Transform*> childrens;

};

#endif