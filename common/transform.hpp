#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>
#include <vector>

class Transform {

    public: 

        Transform(Transform* parent = nullptr);
        Transform(const glm::vec3& _s, const glm::mat3& _r, const glm::vec3& _t, Transform* _parent = nullptr);
        Transform(const Transform& _transform, Transform* _parent = nullptr);

        //Getter

        glm::vec3 getLocalScale() const { return s; }   
        glm::mat3 getLocalRotation() const { return r; }
        glm::vec3 getLocalTranslation() const { return t; }

        glm::vec3 getWorldScale() const; 
        glm::mat3 getWorldRotation() const;
        glm::vec3 getWorldTranslation() const;

        const Transform* getParent() const { return parent; }
        Transform* getParent() { return parent; }
    
        const std::vector<Transform*> getAllChildren() const { return children; }
        std::vector<Transform*> getAllChildren() { return children; }
        
        //Setter
        
        void setScale(const glm::vec3& _s) { s = _s; }
        void setRotation(const glm::mat3& _r) { r = _r; }
        void setTranslation(const glm::vec3& _t) { t = _t; }

        //Other

        void addChild(Transform* _child);
        void removeChild(Transform* _child);
        void removeAllChildren();
        void setParent(Transform* _parent);

        //Modifier 

        void scale(const glm::vec3& _s);
        void rotate(const glm::mat3& _r);
        void translate(const glm::vec3& _t);

        void rotateAround(const Transform _transform);

    private:
        
        glm::vec3 s{1.f}; // uniform scale
        glm::mat3 r{0.f}; // rotation matrix
        glm::vec3 t{0.f}; // translation vector

        Transform* parent;
        std::vector<Transform*> children;
    
};

#endif