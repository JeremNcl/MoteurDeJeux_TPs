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
glm::vec3 sunCenter = glm::vec3(0.0f); // center of the sun
float sunRotationSpeed = 5.0f; // degrees per second
float sunRotationAngle = 0.0f; // current angle in degrees
float sunRotationAngleRad = 0.0f; // current angle in radians
glm::vec3 sunRotation = glm::vec3(0.0f); // current rotation for sun

float earthOrbitSpeed = 20.0f; // degrees per second
float earthOrbitAngle = 0.0f; // current orbit angle in degrees
float earthOrbitAngleRad = 0.0f; // current orbit angle in radians
glm::vec3 earthOrbitRotation = glm::vec3(0.0f); // current rotation for orbit

float earthAxialTilt = glm::radians(23.5f); // axial tilt of the Earth in radians
float earthRotationSpeed = 40.0f; // degrees per second
float earthRotationAngle = 0.0f; // current rotation angle in degrees
float earthRotationAngleRad = 0.0f; // current rotation angle in radians
glm::vec3 earthRotation = glm::vec3(0.0f); // current rotation for earth

float moonOrbitSpeed = 60.0f; // degrees per second
float moonOrbitAngle = 0.0f; // current orbit angle in degrees
float moonOrbitAngleRad = 0.0f; // current orbit angle in radians
glm::vec3 moonOrbitRotation = glm::vec3(0.0f); // current rotation for orbit

float moonAxialTilt = glm::radians(6.68f); // axial tilt of the Moon in radians
float moonRotationSpeed = 20.0f; // degrees per second
float moonRotationAngle = 0.0f; // current rotation angle in degrees
float moonRotationAngleRad = 0.0f; // current rotation angle in radians
glm::vec3 moonRotation = glm::vec3(0.0f); // current rotation for moon


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
    glClearColor(0.f, 0.f, 0.05f, 0.0f);

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
        glm::vec3(0.0f, 0.0f, 9.0f),      // position
        glm::vec3(0.0f, 0.0f, 0.0f),      // target (regard vers le centre du soleil)
        glm::vec3(0.0f, 1.0f, 0.0f),      // up
        5.0f                              // speed
    );
    camera.setMode(FIXED_CAMERA, window);
    
    // === CONSTRUCTION DU GRAPHE DE SCÈNE ===
    SceneGraph sceneGraph;

    // Chargement des textures
    GLuint sunTexture = loadBMP_custom("textures/sun8k.bmp");
    GLuint earthTexture = loadBMP_custom("textures/earth.bmp");
    GLuint moonTexture = loadBMP_custom("textures/moon.bmp");
    
    // === CONSTRUCTION DES NŒUDS DE LA SCÈNE ===
    // Soleil
    auto sunMesh = Mesh::generateSphere(1.0f, 32, 16);
    auto sunNode = std::make_shared<MeshNode>("Sun", sunMesh);
    sunNode->setShaderProgram(programID);
    sunNode->setTexture(sunTexture);
    sceneGraph.getRoot()->addChild(sunNode);

    // Orbite de la Terre
    auto earthOrbitNode = std::make_shared<SceneNode>("Earth orbit");
    sceneGraph.getRoot()->addChild(earthOrbitNode);

    // Terre
    auto earthMesh = Mesh::generateSphere(1.0f, 32, 16); 
    auto earthNode = std::make_shared<MeshNode>("Earth", earthMesh);
    earthNode->setShaderProgram(programID);
    earthNode->setTexture(earthTexture);
    earthOrbitNode->addChild(earthNode);

    // Initialisation de la Terre
    earthOrbitNode->getTransform().setTranslation(glm::vec3(5.0f, 0.0f, 0.0f));
    earthOrbitNode->getTransform().scale(glm::vec3(0.3f));

    // Orbite de la Lune
    auto moonOrbitNode = std::make_shared<SceneNode>("Moon orbit");
    earthOrbitNode->addChild(moonOrbitNode);

    // Lune
    auto moonMesh = Mesh::generateSphere(1.0f, 32, 16);
    auto moonNode = std::make_shared<MeshNode>("Moon", moonMesh);
    moonNode->setShaderProgram(programID);
    moonNode->setTexture(moonTexture);
    moonOrbitNode->addChild(moonNode);

    // Initialisation de la Lune
    moonOrbitNode->getTransform().scale(glm::vec3(0.3f));
    moonOrbitNode->getTransform().setTranslation(glm::vec3(2.f, 0.0f, 0.0f));

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
        earthOrbitNode->getTransform().setRotation(earthOrbitRotation);

        // Rotation de la Terre sur elle-même
        earthRotationAngle += deltaTime * earthRotationSpeed;
        earthRotationAngleRad = glm::radians(earthRotationAngle);
        earthRotation = glm::vec3(earthAxialTilt, earthRotationAngleRad, 0.0f);
        earthNode->getTransform().setRotation(earthRotation);

        // Orbite de la Lune autour de la Terre
        moonOrbitAngle += deltaTime * moonOrbitSpeed;
        moonOrbitAngleRad = glm::radians(moonOrbitAngle);
        moonOrbitRotation = glm::vec3(0.0f, moonOrbitAngleRad, 0.0f);
        moonOrbitNode->getTransform().setRotation(moonOrbitRotation);

        // Rotation de la Lune sur elle même
        moonRotationAngle += deltaTime * moonRotationSpeed;
        moonRotationAngleRad = glm::radians(moonRotationAngle);
        moonRotation = glm::vec3(moonAxialTilt, moonRotationAngleRad, 0.0f);
        moonNode->getTransform().setRotation(moonRotation);
        

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

    // Changement de mode caméra avec la touche C
    static bool cKeyWasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cKeyWasPressed) {
        cKeyWasPressed = true;
        if (camera.getMode() == FIXED_CAMERA) {
            camera.setMode(FREE_CAMERA, window);
            printf("Mode: FREE_CAMERA - Utilisez WASD pour vous déplacer et la souris pour regarder\n");
        } else if (camera.getMode() == FREE_CAMERA) {
            camera.setMode(FIXED_CAMERA, window);
            printf("Mode: FIXED_CAMERA - Vue isométrique fixe\n");
        } 
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        cKeyWasPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
