#ifndef MESHNODEUTILS_HPP
#define MESHNODEUTILS_HPP

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "meshNode.hpp"
#include "../terrain/terrainUtils.hpp"
#include "../terrain/terrain.hpp"

namespace MeshNodeUtils {

    inline void handleBunnyMovement(
        GLFWwindow* window,
        std::shared_ptr<MeshNode> bunny,
        const Terrain& terrain,
        float deltaTime
    ) 
    {
        glm::vec3 currentPos = glm::vec3(bunny->getTransform().getWorldMatrix()[3]);
        glm::vec3 moveDir(0.0f);
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    moveDir.z -= 1.0f; // Avancer
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  moveDir.z += 1.0f; // Reculer
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  moveDir.x -= 1.0f; // Gauche
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) moveDir.x += 1.0f; // Droite

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);
            
            float speed = 150.0f;
            glm::vec3 nextPos = currentPos + moveDir * speed * deltaTime;

            float groundHeight = TerrainUtils::getHeightAt(
                terrain.getHeightmap(),
                terrain.getMaxHeight(),
                terrain.getResolution(),
                nextPos.x,
                nextPos.z
            );

            nextPos.y = groundHeight + 0.5f; // offset de 0.5f pour eviter d'être dans le terrain
            bunny->getTransform().setTranslation(nextPos);
        }
    }

}
#endif