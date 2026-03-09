// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cmath>
#include <random>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

using namespace glm;

#include <common/render/shader.hpp>
#include <common/io/textureLoader.hpp>
#include <common/scene/camera.hpp>
#include <common/scene/SceneGraph.hpp>
#include <common/scene/MeshNode.hpp>

void processInput(GLFWwindow *window, Camera& camera);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 0.;
float zoom = 1.;
/*******************************************************************************/


int main( void )
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP1 - GLFW", NULL, NULL);
    if( window == NULL ){
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Register callback for window resize
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");

    // Handle MVP matrix uniform
    glUseProgram(programID);
    GLuint MVP = glGetUniformLocation(programID, "MVP");
    
    // Configurer la caméra simple
    Camera camera;
    camera.initialize(
        glm::vec3(0.0f, 2.0f, 4.0f),      // position
        glm::vec3(0.0f, 0.0f, 0.0f),      // target (regard vers la sphère)
        glm::vec3(0.0f, 1.0f, 0.0f),      // up
        5.0f                               // speed
    );
    camera.setMode(FIXED_CAMERA, window);
    
    // === CONSTRUCTION DU GRAPHE DE SCÈNE ===
    SceneGraph sceneGraph;

    // Charger une texture pour la sphère
    GLuint grassTexture = loadBMP_custom("textures/grass.bmp");
    
    // === SPHÈRE ===
    auto sphereNode = MeshNode::loadFromOFF("meshes/sphere.off", programID, "Sphere", true);
    if (sphereNode) {
        sphereNode->setTexture(grassTexture);
        sphereNode->getTransform().setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        sphereNode->getTransform().setUniformScale(1.0f);
        sceneGraph.getRoot()->addChild(sphereNode);
    } else {
        printf("ERREUR: Impossible de charger sphere.off\n");
        return -1;
    }
    
    printf("Graphe de scène initialisé avec %d nœud(s)\n", sceneGraph.getNodeCount());

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint lightID = glGetUniformLocation(programID, "LightPosition_worldspace");


    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{

        // Measure speed
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Inputs
        processInput(window, camera); // terrainNode.get()

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Mettre à jour la caméra
        camera.update(window, deltaTime);
        
        // Calculer la matrice View-Projection
        glm::mat4 viewProjection = camera.getProjectionMatrix() * camera.getViewMatrix();
        
        // Mettre à jour et dessiner toute la scène via le graphe de scène
        sceneGraph.update(deltaTime);
        sceneGraph.draw(viewProjection);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup
    MeshNode::clearMeshCache();
    glDeleteTextures(1, &grassTexture);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// Gestion des inputs
void processInput(GLFWwindow *window, Camera& camera)
{
    // Fermer la fenêtre (touche ESC)
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Retour à la caméra fixe (touche F)
    static bool fKeyWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fKeyWasPressed) {
        fKeyWasPressed = true;
        camera.setMode(FIXED_CAMERA, window);
        printf("Mode: FIXED_CAMERA - Vue isométrique fixe\n");
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fKeyWasPressed = false;
    }

    // Ajuster la vitesse de la caméra orbitale (touches UP/DOWN)
    if (camera.getMode() == ORBIT_CAMERA) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            float newSpeed = camera.getOrbitSpeed() + 2.0f * deltaTime;
            camera.setOrbitSpeed(newSpeed);
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            float newSpeed = camera.getOrbitSpeed() - 2.0f * deltaTime;
            camera.setOrbitSpeed(newSpeed);
        }
    }

    // Changement de mode caméra avec la touche C
    static bool cKeyWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cKeyWasPressed) {
        cKeyWasPressed = true;
        if (camera.getMode() == FIXED_CAMERA) {
            camera.setMode(FREE_CAMERA, window);
            printf("Mode: FREE_CAMERA - Utilisez WASD pour vous déplacer et la souris pour regarder\n");
        } else if (camera.getMode() == FREE_CAMERA) {
            camera.setMode(ORBIT_CAMERA, window);
            printf("Mode: ORBIT_CAMERA - Rotation automatique autour du terrain\n");
        } else if (camera.getMode() == ORBIT_CAMERA) {
            camera.setMode(FREE_CAMERA, window);
            printf("Mode: FREE_CAMERA - Utilisez WASD pour vous déplacer et la souris pour regarder\n");
        }
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        cKeyWasPressed = false;
    }
    
    // // Augmenter la résolution (touches + ou KP_ADD)
    // static bool plusKeyWasPressed = false;
    // if ((glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || 
    //      glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) && !plusKeyWasPressed) {
    //     plusKeyWasPressed = true;
    //     float currentRes = terrainNode->getTerrain().getResolution();
    //     float newRes = currentRes / 2.0f;
    //     terrainNode->getTerrain().setResolution(newRes);
    //     terrainNode->regenerateMesh();
    // }
    // if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_RELEASE && 
    //     glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_RELEASE) {
    //     plusKeyWasPressed = false;
    // }
    
    // // Diminuer la résolution (touches - ou KP_SUBTRACT)
    // static bool minusKeyWasPressed = false;
    // if ((glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || 
    //      glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) && !minusKeyWasPressed) {
    //     minusKeyWasPressed = true;
    //     float currentRes = terrainNode->getTerrain().getResolution();
    //     float newRes = currentRes * 2.0f;
    //     terrainNode->getTerrain().setResolution(newRes);
    //     terrainNode->regenerateMesh();
    // }
    // if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_RELEASE && 
    //     glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_RELEASE) {
    //     minusKeyWasPressed = false;
    // }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
