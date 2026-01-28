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

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <common/texture.hpp>

void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Terrain dimensions
int terrain_width = 16;
int terrain_height = 16;

// camera
glm::vec3 camera_position   = glm::vec3(20.0f, 15.0f, 20.0f);
glm::vec3 camera_target     = glm::vec3(7.5f, 0.0f, 7.5f);
glm::vec3 camera_up         = glm::vec3(0.0f, 1.0f, 0.0f);

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//rotation
float angle = 0.;
float zoom = 1.;
/*******************************************************************************/

// Configure la caméra pour voir tout le terrain en vue isométrique
void setupCameraForTerrain(int width, int height)
{
    // Centre du terrain
    float centerX = width / 2.0f;
    float centerZ = height / 2.0f;
    
    // Distance de la caméra pour voir tout le terrain
    // On utilise la plus grande dimension et on ajoute une marge
    float maxDim = std::max(width, height);
    float distance = maxDim * 1.2f;  // Distance augmentée pour voir tout le terrain
    
    // Position de la caméra en vue isométrique (45° en horizontal et en vertical)
    camera_position = glm::vec3(
        centerX + distance * 0.7f,  // Offset en X
        distance * 0.6f,             // Hauteur augmentée
        centerZ + distance * 0.7f    // Offset en Z
    );
    
    // La caméra regarde le centre du terrain
    camera_target = glm::vec3(centerX, 0.0f, centerZ);
    
    // Vecteur up standard
    camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    printf("Camera configured for terrain %dx%d\n", width, height);
    printf("Camera position: (%.1f, %.1f, %.1f)\n", camera_position.x, camera_position.y, camera_position.z);
    printf("Camera target: (%.1f, %.1f, %.1f)\n", camera_target.x, camera_target.y, camera_target.z);
}

// Charge une heightmap depuis un fichier BMP
// Retourne un vecteur 2D avec les hauteurs (0.0 à 1.0)
std::vector<std::vector<float>> loadHeightmap(const char* filepath, int& width, int& height)
{
    printf("Loading heightmap: %s\n", filepath);
    
    unsigned char header[54];
    FILE* file = fopen(filepath, "rb");
    
    if (!file) {
        printf("Heightmap file not found: %s\n", filepath);
        width = 0;
        height = 0;
        return std::vector<std::vector<float>>();
    }
    
    // Lire l'en-tête BMP
    if (fread(header, 1, 54, file) != 54) {
        printf("Invalid BMP file\n");
        fclose(file);
        return std::vector<std::vector<float>>();
    }
    
    // Vérifier la signature BMP
    if (header[0] != 'B' || header[1] != 'M') {
        printf("Not a BMP file\n");
        fclose(file);
        return std::vector<std::vector<float>>();
    }
    
    // Extraire les dimensions
    width = *(int*)&(header[0x12]);
    height = *(int*)&(header[0x16]);
    int dataPos = *(int*)&(header[0x0A]);
    int imageSize = *(int*)&(header[0x22]);
    int bpp = *(short*)&(header[0x1C]);  // Bits per pixel
    
    // Corrections pour les BMP malformés
    if (imageSize == 0) imageSize = width * height * (bpp / 8);
    if (dataPos == 0) dataPos = 54;
    
    printf("Heightmap size: %d x %d, %d bpp\n", width, height, bpp);
    
    // Lire les données d'image
    unsigned char* data = new unsigned char[imageSize];
    fseek(file, dataPos, SEEK_SET);
    fread(data, 1, imageSize, file);
    fclose(file);
    
    // Convertir les données en heightmap (normaliser entre 0 et 1)
    std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));
    int bytesPerPixel = bpp / 8;
    
    // Stocker les dimensions globalement
    terrain_width = width;
    terrain_height = height;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixelIndex = (y * width + x) * bytesPerPixel;
            // Pour une image en niveaux de gris, prendre le premier canal
            unsigned char pixelValue = data[pixelIndex];
            // Normaliser entre 0 et 1
            heightmap[y][x] = (float)pixelValue / 255.0f;
        }
    }
    
    delete[] data;
    printf("Heightmap loaded successfully\n");
    return heightmap;
}

void initPlane(std::vector<vec3> &vertices, std::vector<unsigned int> &indices, std::vector<vec2> &uvs, std::vector<vec3> &normals)
{
    int hmWidth, hmHeight;
    std::vector<std::vector<float>> heightmap = loadHeightmap("heightmaps/heightmap-1024x1024.bmp", hmWidth, hmHeight);
    
    if (heightmap.empty()) {
        printf("Failed to load heightmap, using flat plane\n");
        hmWidth = 16;
        hmHeight = 16;
    }
    
    int width = hmWidth;
    int height = hmHeight;
    float maxHeight = 50.0f;  // Hauteur maximale du terrain

    vertices.clear();
    indices.clear();
    uvs.clear();
    normals.clear();

    // Création des sommets et des uvs
    for (int z = 0; z < height; z++) {
        for (int x = 0; x < width; x++) {
            vec3 v;
            v.x = (float)x;
            
            // Utiliser la heightmap pour la hauteur
            if (!heightmap.empty() && z < (int)heightmap.size() && x < (int)heightmap[z].size()) {
                v.y = heightmap[z][x] * maxHeight;
            } else {
                v.y = 0.0f;
            }
            
            v.z = (float)z;
            vertices.push_back(v);
            
            vec2 uv;
            uv.x = x / float(width - 1);
            uv.y = z / float(height - 1);
            uvs.push_back(uv);
            
            // Initialiser les normales à zéro (seront calculées après)
            normals.push_back(vec3(0.0f, 0.0f, 0.0f));
        }
    }

    // Indexation des triangles
    for (unsigned int j = 0; j < height; j++) {
        for (unsigned int i = 0; i < width; i++) {
            if (i+1 < width && j+1 < height) {
                // Triangle 1
                indices.push_back(i + j*width);       // TOP_LEFT
                indices.push_back(i + (j+1)*width);   // BOTTOM_LEFT
                indices.push_back(i+1 + j*width);     // TOP_RIGHT
                // Triangle 2
                indices.push_back(i+1 + j*width);     // TOP_RIGHT
                indices.push_back(i + (j+1)*width);   // BOTTOM_LEFT
                indices.push_back(i+1 + (j+1)*width); // BOTTOM_RIGHT
            }
        }
    }
    
    // Calcul des normales
    // Pour chaque triangle, calculer la normale et l'ajouter aux normales des sommets
    for (size_t i = 0; i < indices.size(); i += 3) {
        unsigned int id0 = indices[i];
        unsigned int id1 = indices[i + 1];
        unsigned int id2 = indices[i + 2];
        
        vec3 v0 = vertices[id0];
        vec3 v1 = vertices[id1];
        vec3 v2 = vertices[id2];
        
        // Calculer les vecteurs des arêtes du triangle
        vec3 edge1 = v1 - v0;
        vec3 edge2 = v2 - v0;
        
        // Normale du triangle (produit vectoriel)
        vec3 triangleNormal = glm::normalize(glm::cross(edge1, edge2));
        
        // Ajouter cette normale aux trois sommets du triangle
        normals[id0] += triangleNormal;
        normals[id1] += triangleNormal;
        normals[id2] += triangleNormal;
    }
    
    // Normaliser toutes les normales des sommets
    for (size_t i = 0; i < normals.size(); i++) {
        normals[i] = glm::normalize(normals[i]);
    }
    
    printf("Normals calculated for %zu vertices\n", normals.size());
}


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

    /*****************TODO***********************/
    // Get a handle for our "Model View Projection" matrices uniforms
    glUseProgram(programID);
    GLuint MVP = glGetUniformLocation(programID, "MVP");

    /****************************************/
    std::vector<unsigned int> indices; //Triangles concaténés dans une liste
    //std::vector<std::vector<unsigned int> > triangles;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    // //Chargement du fichier de maillage
    // std::string filename("chair.off");
    // loadOFF(filename, indexed_vertices, indices, triangles );

    // Initialisation du plan
    initPlane(indexed_vertices, indices, uvs, normals);
    
    // Configure la caméra automatiquement pour voir tout le terrain
    setupCameraForTerrain(terrain_width, terrain_height);

    // Load it into a VBO
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0] , GL_STATIC_DRAW);

    // Buffer for the UVs
    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0] , GL_STATIC_DRAW);

    // Buffer for the normals
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

    // Load texture
    GLuint texture = loadBMP_custom("textures/grass.bmp");
    GLuint textureID = glGetUniformLocation(programID, "diffuseTexture");

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

        // input
        // -----
        processInput(window);


        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureID, 0);

        /*****************TODO***********************/
        // Model matrix : an identity matrix (model will be at the origin) then change
        glm::mat4 modelMat = glm::mat4(1.0f);

        // View matrix : camera/view transformation lookat() utiliser camera_position camera_target camera_up
        glm::mat4 viewMat = glm::lookAt(
            camera_position,
            camera_target,
            camera_up
        );

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 5000 units
        float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
        glm::mat4 projectionMat = glm::perspective(
            glm::radians(45.0f),
            aspectRatio,          
            0.1f,
            5000.0f
        );

        // Send our transformation to the currently bound shader,
        // in the "Model View Projection" to the shader uniforms
        glm::mat4 MVPMat = projectionMat * viewMat * modelMat;
        glUniformMatrix4fv(MVP, 1, GL_FALSE, &MVPMat[0][0]);

        /****************************************/


        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
        );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // 2nd attribute buffer : uvs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
                    1,                  // attribute
                    2,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
        );

        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                    2,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
        );

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_INT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteTextures(1, &texture);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    //Camera zoom in and out
    float cameraSpeed = 2.5 * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_position += cameraSpeed * camera_target;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_position -= cameraSpeed * camera_target;

    //TODO add translations

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
