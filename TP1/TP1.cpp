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
#include <common/scene/sceneGraph.hpp>
#include <common/scene/meshNode.hpp>


void processInput(GLFWwindow *window, Camera& camera);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// planets parameters
float sunRotationSpeed = 10.0f; // degrees per second
float sunRotationAngle = 0.0f; // current angle in degrees
float sunRotationAngleRad = 0.0f; // current angle in radians
glm::vec3 sunRotation = glm::vec3(0.0f); // current rotation for sun
glm::vec3 sunCenter = glm::vec3(0.0f); // center of the sun

glm::vec3 earthInitialPosition;
float earthOrbitSpeed = 30.0f; // degrees per second
float earthOrbitAngle = 0.0f; // current orbit angle in degrees
float earthOrbitAngleRad = 0.0f; // current orbit angle in radians
glm::vec3 earthOrbitRotation = glm::vec3(0.0f); // current rotation for orbit

float earthRotationSpeed = 60.0f; // degrees per second
float earthRotationAngle = 0.0f; // current rotation angle in degrees
float earthRotationAngleRad = 0.0f; // current rotation angle in radians
glm::vec3 earthRotation = glm::vec3(0.0f); // current rotation for earth

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

    // Chargement des textures
    GLuint sunTexture = loadBMP_custom("textures/sun.bmp");
    GLuint earthTexture = loadBMP_custom("textures/earth.bmp");
    
    // === CONSTRUCTION DES NŒUDS DE LA SCÈNE ===
    // Soleil
    auto sunMesh = Mesh::generateSphere(1.0f, 32, 16); // rayon, méridiens, parallèles
    auto sunNode = std::make_shared<MeshNode>("Sun", sunMesh);
    sunNode->setShaderProgram(programID);
    sunNode->setTexture(sunTexture);
    sceneGraph.getRoot()->addChild(sunNode);

    // Terre-Lune
    auto earthMoonNode = std::make_shared<SceneNode>("EarthMoon");
    sceneGraph.getRoot()->addChild(earthMoonNode);

    // Terre
    auto earthMesh = Mesh::generateSphere(1.0f, 32, 16); // rayon, méridiens, parallèles
    auto earthNode = std::make_shared<MeshNode>("Earth", earthMesh);
    earthNode->setShaderProgram(programID);
    earthNode->setTexture(earthTexture);
    earthMoonNode->addChild(earthNode);

    // Initialisation : position sur l'orbite (rayon constant)
    earthMoonNode->getTransform().setTranslation(glm::vec3(3.0f, 0.0f, 0.0f));
    earthMoonNode->getTransform().scale(glm::vec3(0.3f));
    earthInitialPosition = earthNode->getTransform().getWorldTranslation();
    // Inclinaison de l'axe de rotation de la Terre
    earthNode->getTransform().rotate(glm::vec3(0.0f, glm::radians(23.5f), 0.0f)); 

    
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

        // === Déplacer les objets de la scène ===
        // Rotation du soleil
        sunRotationAngle += deltaTime * sunRotationSpeed;
        sunRotationAngleRad = glm::radians(sunRotationAngle);
        sunRotation = glm::vec3(0.0f, sunRotationAngleRad, 0.0f);
        sunNode->getTransform().setRotation(sunRotation);
        
        // Orbite de la Terre autour du Soleil
        earthOrbitAngle += deltaTime * earthOrbitSpeed;
        earthOrbitAngleRad = glm::radians(earthOrbitAngle);
        earthOrbitRotation = glm::vec3(0.0f, earthOrbitAngleRad, 0.0f);
        earthMoonNode->getTransform().setRotationAround(earthOrbitRotation, sunCenter, earthInitialPosition);

        // Rotation de la Terre sur elle-même
        earthRotationAngle += deltaTime * earthRotationSpeed;
        earthRotationAngleRad = glm::radians(earthRotationAngle);
        earthRotation = glm::vec3(0.0f, earthRotationAngleRad, 0.0f);
        earthNode->getTransform().setRotation(earthRotation);
        

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
    glDeleteTextures(1, &sunTexture);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// Gestion des inputs
void processInput(GLFWwindow *window, Camera& camera) {
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
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
