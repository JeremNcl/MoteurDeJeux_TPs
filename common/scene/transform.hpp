#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include <glm/glm.hpp>

#include <vector>


class Transform {

    public: 

        Transform(Transform* parent = nullptr);
        Transform(const glm::vec3& _s, const glm::mat3& _r, const glm::vec3& _t, Transform* _parent = nullptr);
        Transform(const Transform& _transform, Transform* _parent = nullptr);
        ~Transform();

        //Getter

        glm::vec3 getLocalScale() const { return s; }   
        glm::mat3 getLocalRotation() const { return r; }
        glm::vec3 getLocalTranslation() const { return t; }

        glm::vec3 getWorldScale() const; 
        glm::mat3 getWorldRotation() const;
        glm::vec3 getWorldTranslation() const;

        glm::mat4 getWorldMatrix() const;

        const Transform* getParent() const { return parent; }
        Transform* getParent() { return parent; }
    
        const std::vector<Transform*> getAllChildren() const { return children; }
        std::vector<Transform*> getAllChildren() { return children; }
        
        /**
         * @brief Définit le scale local (remplace la valeur).
         * @param _s Nouveau vecteur de scale.
         */
        void setScale(const glm::vec3& _s) { s = _s; }

        /**
         * @brief Définit la rotation locale (remplace la valeur).
         * @param _r Nouvelle matrice de rotation.
         */
        void setRotation(const glm::mat3& _r) { r = _r; }

        /**
         * @brief Définit la translation locale (remplace la valeur).
         * @param _t Nouveau vecteur de translation.
         */
        void setTranslation(const glm::vec3& _t) { t = _t; }

        /**
         * @brief Ajoute un enfant à ce Transform.
         * @param _child Pointeur vers le Transform enfant.
         */
        void addChild(Transform* _child);

        /**
         * @brief Retire un enfant de ce Transform.
         * @param _child Pointeur vers le Transform enfant à retirer.
         */
        void removeChild(Transform* _child);

        /**
         * @brief Retire tous les enfants de ce Transform.
         */
        void removeAllChildren();

        /**
         * @brief Définit le parent de ce Transform.
         * @param _parent Pointeur vers le Transform parent.
         */
        void setParent(Transform* _parent);

        /**
         * @brief Applique un scale multiplicatif local.
         * @param _s Vecteur de scale à multiplier.
         */
        void scale(const glm::vec3& _s);

        /**
         * @brief Applique une rotation locale (composition).
         * Convention : la nouvelle rotation _r est appliquée avant la rotation actuelle (pré-multiplication).
         * @param _r Matrice de rotation à composer.
         */
        void rotate(const glm::mat3& _r);

        /**
         * @brief Applique une translation locale (addition).
         * @param _t Vecteur de translation à ajouter.
         */
        void translate(const glm::vec3& _t);

        // ajouter rotateAround si la hiérarchie parent/enfant ne suffit plus

    private:
        
        glm::vec3 s{1.f}; // uniform scale
        glm::mat3 r{0.f}; // rotation matrix
        glm::vec3 t{0.f}; // translation vector

        Transform* parent;
        std::vector<Transform*> children;
    
};

#endif