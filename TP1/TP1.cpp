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
#include <common/terrain/terrainNode.hpp>


void processInput(GLFWwindow *window, Camera& camera);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

/*******************************************************************************/


int main( void ) {
    
    // Initialisation de GLFW
    if( !glfwInit() ) {
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

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Light blue background
    glClearColor(0.39f, 0.65f, 0.85f, 0.85f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    // glEnable(GL_CULL_FACE);

    // Create and compile our GLSL program from the shaders
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint meshesProgramID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    GLuint terrainProgramID = LoadShaders("vertex_shader.glsl", "terrain_shader.glsl");

    // Handle MVP matrix uniform
    glUseProgram(meshesProgramID);
    GLuint meshesMVP = glGetUniformLocation(meshesProgramID, "MVP");
    glUseProgram(terrainProgramID);
    GLuint terrainMVP = glGetUniformLocation(terrainProgramID, "MVP");

    
    // === INITIALISATION DE LA SCÈNE ===
    
    // Chargement des textures
    GLuint grassTexture = loadBMP_custom("textures/grass.bmp");
    GLuint rockTexture = loadBMP_custom("textures/rock.bmp");
    GLuint snowTexture = loadBMP_custom("textures/snowrocks.bmp");
    GLuint sunTexture = loadBMP_custom("textures/sun8k.bmp");
    
    // Construction du graphe de scène
    SceneGraph sceneGraph;

    // Création du terrain
    Terrain terrain = Terrain();
    terrain.loadHeightmap("heightmaps/heightmap_mountain.bmp", 120.0f);
    // Génération du mesh du terrain 
    auto terrainMesh = std::make_shared<Mesh>();
    terrain.generateMesh(*terrainMesh);
    // Créer du nœud du terrain
    std::shared_ptr<TerrainNode> terrainNode = std::make_shared<TerrainNode>("terrain", terrain, terrainMesh);
    terrainNode->setShaderProgram(terrainProgramID);
    terrainNode->setTextures(grassTexture, rockTexture, snowTexture);
    terrainNode->setHeightParameters(40.0f, 100.0f, 10.0f); 
    // Ajout au graphe de scène
    sceneGraph.getRoot()->addChild(terrainNode);

    // Création du mesh du lapin
    auto bunnyMesh = Mesh::loadFromOFF("meshes/suzan.off");
    // Création du nœud du lapin
    auto bunnyNode = std::make_shared<MeshNode>("Bunny", bunnyMesh);
    bunnyNode->setShaderProgram(meshesProgramID);
    bunnyNode->setTexture(sunTexture);
    // Positionner le lapin au centre du terrain
    glm::vec3 terrainCenter = terrain.getCenterPosition();
    bunnyNode->getTransform().setTranslation(terrain.getCenterPosition());
    bunnyNode->getTransform().setScale(glm::vec3(3.0f));
    // Ajout au graphe de scène
    sceneGraph.getRoot()->addChild(bunnyNode);

    printf("Graphe de scène initialisé avec %d nœud(s)\n", sceneGraph.getNodeCount());

    // Initialisation de la caméra
    Camera camera;
    CameraSetup cameraSetup = terrain.getOptimalIsometricView();
    camera.initialize(
        cameraSetup.position,
        cameraSetup.target,
        cameraSetup.up,
        cameraSetup.speed// 50.0f
    );
    camera.setMode(FIXED_CAMERA, window);


    // // Get a handle for our "LightPosition" uniform
    // glUseProgram(terrainProgramID);
    // GLuint lightID = glGetUniformLocation(terrainProgramID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{
        // Measure speed
        // per-frame time logic
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
        
        

        // Mettre à jour et dessiner toute la scène via le graphe de scène
        sceneGraph.update(deltaTime);
        sceneGraph.draw(viewProjection);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS) && (glfwWindowShouldClose(window) == 0) );

    // Cleanup
    MeshNode::clearMeshCache();
    // delete textures
    glDeleteProgram(meshesProgramID);
    glDeleteProgram(terrainProgramID);

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
