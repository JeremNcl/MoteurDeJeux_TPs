#ifndef MESH_HPP
#define MESH_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Mesh {
public:
    Mesh();
    ~Mesh();

    static std::shared_ptr<Mesh> loadFromOFF(const std::string& filename, bool enableCache = true);
    static void clearMeshCache();

    void computeNormals();
    void computeUVs();

    void uploadToGPU();
    void releaseGPU();

    bool hasGPUData() const;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    GLuint vertexBuffer;
    GLuint indexBuffer;
    GLuint uvBuffer;
    GLuint normalBuffer;
    size_t indexCount;

    GLuint texture;
    GLuint shaderProgram;

private:
    static std::unordered_map<std::string, std::shared_ptr<Mesh>> meshCache;
};

#endif
