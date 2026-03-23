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

    // Light blue background
    glClearColor(0.39f, 0.65f, 0.85f, 0.85f);

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
    GLuint sunProgramID = LoadShaders("vertex_shader.glsl", "fragment_shader.glsl");
    GLuint terrainProgramID = LoadShaders("vertex_shader.glsl", "terrain_shader.glsl");

    // Handle MVP matrix uniform
    glUseProgram(sunProgramID);
    GLuint sunMVP = glGetUniformLocation(sunProgramID, "MVP");
    glUseProgram(terrainProgramID);
    GLuint terrainMVP = glGetUniformLocation(terrainProgramID, "MVP");

    // Configurer la caméra simple
    Camera camera;
    // camera.initialize(
    //     glm::vec3(0.0f, 0.0f, 9.0f),      // position
    //     glm::vec3(0.0f, 0.0f, 0.0f),      // target (regard vers le centre du soleil)
    //     glm::vec3(0.0f, 1.0f, 0.0f),      // up
    //     5.0f                              // speed
    // );
    // camera.setMode(FIXED_CAMERA, window);
    
    // === CONSTRUCTION DU GRAPHE DE SCÈNE ===
    SceneGraph sceneGraph;

    // Chargement des textures
    GLuint grassTexture = loadBMP_custom("textures/grass.bmp");
    GLuint rockTexture = loadBMP_custom("textures/rock.bmp");
    GLuint snowTexture = loadBMP_custom("textures/snowrocks.bmp");
    GLuint sunTexture = loadBMP_custom("textures/sun8k.bmp");
    
    // === CONSTRUCTION DES NŒUDS DE LA SCÈNE ===
    // Soleil pour être sûr que ça marche
    auto sunMesh = Mesh::generateSphere(1.0f, 32, 16);
    auto sunNode = std::make_shared<MeshNode>("Sun", sunMesh);
    sunNode->setShaderProgram(sunProgramID);
    sunNode->setTexture(sunTexture);
    // Placer le soleil au centre du terrain
    sunNode->getTransform().setTranslation(glm::vec3(0, 50, 0)); // Y=50 pour qu'il soit au-dessus du terrain
    sceneGraph.getRoot()->addChild(sunNode);

    // Génération du terrain
    Terrain terrain = Terrain();
    terrain.loadHeightmap("heightmaps/heightmap_mountain.bmp"); 
    terrain.setResolution(1.0f); 

    // === DEBUG TERRAIN ===
    printf("[DEBUG TERRAIN] Taille du terrain : %d x %d\n", terrain.getWidth(), terrain.getHeight());
    // Afficher quelques valeurs de la heightmap (coin haut-gauche, centre, coin bas-droit)
    if (!terrain.heightmap.empty()) {
        int w = terrain.getWidth();
        int h = terrain.getHeight();
        printf("[DEBUG TERRAIN] Coin haut-gauche : %.3f\n", terrain.heightmap[0][0]);
        printf("[DEBUG TERRAIN] Centre : %.3f\n", terrain.heightmap[h/2][w/2]);
        printf("[DEBUG TERRAIN] Coin bas-droit : %.3f\n", terrain.heightmap[h-1][w-1]);
    } else {
        printf("[DEBUG TERRAIN] Heightmap vide !\n");
    }

    // Génération du mesh du terrain 
    auto terrainMesh = std::make_shared<Mesh>();
    terrain.generateMesh(*terrainMesh);

    // === DEBUG MESH ===
    printf("[DEBUG MESH] Nb vertices : %zu\n", terrainMesh->vertices.size());
    printf("[DEBUG MESH] Nb indices : %zu\n", terrainMesh->indices.size());
    if (!terrainMesh->vertices.empty()) {
        auto& v0 = terrainMesh->vertices[0];
        printf("[DEBUG MESH] Premier vertex : (%.3f, %.3f, %.3f)\n", v0.x, v0.y, v0.z);
        auto& v1 = terrainMesh->vertices[1];
        printf("[DEBUG MESH] Deuxième vertex : (%.3f, %.3f, %.3f)\n", v1.x, v1.y, v1.z);
        auto& v513 = terrainMesh->vertices[513];
        printf("[DEBUG MESH] 513ème vertex : (%.3f, %.3f, %.3f)\n", v513.x, v513.y, v513.z);
    }
    if (!terrainMesh->indices.empty()) {
        printf("[DEBUG MESH] Premiers indices : %u, %u, %u\n", terrainMesh->indices[0], terrainMesh->indices.size() > 1 ? terrainMesh->indices[1] : 0, terrainMesh->indices.size() > 2 ? terrainMesh->indices[2] : 0);
    }
    if (!terrainMesh->uvs.empty()) {
        auto& uv0 = terrainMesh->uvs[0];
        printf("[DEBUG MESH] Premier UV : (%.3f, %.3f)\n", uv0.x, uv0.y);
    }
    if (!terrainMesh->normals.empty()) {
        auto& n0 = terrainMesh->normals[0];
        printf("[DEBUG MESH] Première normale : (%.3f, %.3f, %.3f)\n", n0.x, n0.y, n0.z);
    }
    
    // Créer le nœud de terrain
    std::shared_ptr<TerrainNode> terrainNode = std::make_shared<TerrainNode>("terrain", terrain, terrainMesh);
    terrainNode->setShaderProgram(terrainProgramID);
    terrainNode->setTextures(grassTexture, rockTexture, snowTexture);
    terrainNode->setHeightParameters(40.0f, 100.0f, 10.0f); 
    sceneGraph.getRoot()->addChild(terrainNode);

    // Recentrer le terrain
    // terrainNode->getTransform().setTranslation(glm::vec3(terrain.getWidth() / -2.0f, 0, terrain.getHeight() / -2.0f));
    
    printf("Graphe de scène initialisé avec %d nœud(s)\n", sceneGraph.getNodeCount());


    // Get a handle for our "LightPosition" uniform
    glUseProgram(terrainProgramID);
    GLuint lightID = glGetUniformLocation(terrainProgramID, "LightPosition_worldspace");

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
    glDeleteProgram(sunProgramID);
    glDeleteProgram(terrainProgramID);
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
