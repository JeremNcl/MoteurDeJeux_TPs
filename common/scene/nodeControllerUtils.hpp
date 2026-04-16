#ifndef NODECONTROLLERUTILS_HPP
#define NODECONTROLLERUTILS_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "sceneNode.hpp"

namespace NodeControllerUtils {

    inline void nodeDeplacement(std::shared_ptr<SceneNode> _node, glm::vec3 _direction, float _speed, float _deltaTime)
    {

        // Normalise la direction
        glm::vec3 normalizeDirection = glm::normalize(_direction);

        glm::vec3 currentPos = glm::vec3(_node->getTransform().getWorldMatrix()[3]);
        
        glm::vec3 newPos = currentPos + normalizeDirection * _speed * _deltaTime;
        
        _node->getTransform().setTranslation(newPos);
    }
}

#endif