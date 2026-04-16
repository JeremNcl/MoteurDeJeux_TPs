#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>

#include <glm/glm.hpp>

#include "objectLoader.hpp"


// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

bool loadOFF( const std::string & filename ,
              std::vector< glm::vec3 > & vertices ,
              std::vector< unsigned short > & indices,
              std::vector< std::vector<unsigned short > > & triangles )
{
    bool convertToTriangles = true;
    bool randomize = false;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    std::string magic_s;

    myfile >> magic_s;

    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        return false;
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    vertices.resize(n_vertices);

    for( int v = 0 ; v < n_vertices ; ++v )
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        if( std::isnan(vertex.x) )
            vertex.x = 0.0;
        if( std::isnan(vertex.y) )
            vertex.y = 0.0;
        if( std::isnan(vertex.z) )
            vertex.z = 0.0;
        vertices[v] = vertex;
    }


    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if( n_vertices_on_face == 3 )
        {
            unsigned short _v1 , _v2 , _v3;
            std::vector< unsigned short > _v;
            myfile >> _v1 >> _v2 >> _v3;
            _v.push_back( _v1 );
            _v.push_back( _v2 );
            _v.push_back( _v3 );
            triangles.push_back( _v );
            indices.push_back( _v1 );
            indices.push_back( _v2 );
            indices.push_back( _v3 );

        }
        else if( n_vertices_on_face > 3 )
        {
            std::vector< unsigned short > vhandles;
            vhandles.resize(n_vertices_on_face);
            for( int i=0 ; i < n_vertices_on_face ; ++i )
                myfile >> vhandles[i];

            if( convertToTriangles )
            {
                unsigned short k=(randomize)?(rand()%vhandles.size()):0;
                for (unsigned short i=0;i<vhandles.size()-2;++i)
                {
                    std::vector< unsigned short > tri(3);
                    tri[0]=vhandles[(k+0)%vhandles.size()];
                    tri[1]=vhandles[(k+i+1)%vhandles.size()];
                    tri[2]=vhandles[(k+i+2)%vhandles.size()];
                    triangles.push_back(tri);
                    indices.push_back(tri[0]);
                    indices.push_back(tri[1]);
                    indices.push_back(tri[2]);
                }
            }
            else
            {
                //careful not triangles
                triangles.push_back(vhandles);
                for( int i=0 ; i < vhandles.size() ; ++i )
                    indices.push_back(vhandles[i]);
            }
        }
        else
        {
            std::cout << "OFFIO::open error : Face number " << f << " has " << n_vertices_on_face << " vertices" << std::endl;
            myfile.close();
            return false;
        }
    }

    myfile.close();
    return true;
}

bool loadOFF( const std::string & filename ,
              std::vector< glm::vec3 > & vertices ,
              std::vector< unsigned int > & indices,
              std::vector< std::vector<unsigned int > > & triangles )
{
    bool convertToTriangles = true;
    bool randomize = false;

    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    std::string magic_s;

    myfile >> magic_s;

    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        return false;
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;

    vertices.resize(n_vertices);

    for( int v = 0 ; v < n_vertices ; ++v )
    {
        glm::vec3 vertex;
        myfile >> vertex.x >> vertex.y >> vertex.z;
        if( std::isnan(vertex.x) )
            vertex.x = 0.0;
        if( std::isnan(vertex.y) )
            vertex.y = 0.0;
        if( std::isnan(vertex.z) )
            vertex.z = 0.0;
        vertices[v] = vertex;
    }

    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;
        if( n_vertices_on_face == 3 )
        {
            unsigned int _v1 , _v2 , _v3;
            std::vector< unsigned int > _v;
            myfile >> _v1 >> _v2 >> _v3;
            _v.push_back( _v1 );
            _v.push_back( _v2 );
            _v.push_back( _v3 );
            triangles.push_back( _v );
            indices.push_back( _v1 );
            indices.push_back( _v2 );
            indices.push_back( _v3 );

        }
        else if( n_vertices_on_face > 3 )
        {
            std::vector< unsigned int > vhandles;
            vhandles.resize(n_vertices_on_face);
            for( int i=0 ; i < n_vertices_on_face ; ++i )
                myfile >> vhandles[i];

            if( convertToTriangles )
            {
                unsigned int k=(randomize)?(rand()%vhandles.size()):0;
                for (unsigned int i=0;i<vhandles.size()-2;++i)
                {
                    std::vector< unsigned int > tri(3);
                    tri[0]=vhandles[(k+0)%vhandles.size()];
                    tri[1]=vhandles[(k+i+1)%vhandles.size()];
                    tri[2]=vhandles[(k+i+2)%vhandles.size()];
                    triangles.push_back(tri);
                    indices.push_back(tri[0]);
                    indices.push_back(tri[1]);
                    indices.push_back(tri[2]);
                }
            }
            else
            {
                triangles.push_back(vhandles);
                for( int i=0 ; i < vhandles.size() ; ++i )
                    indices.push_back(vhandles[i]);
            }
        }
        else
        {
            std::cout << "OFFIO::open error : Face number " << f << " has " << n_vertices_on_face << " vertices" << std::endl;
            myfile.close();
            return false;
        }
    }

    myfile.close();
    return true;
}

bool loadOBJ(const std::string& filename,
             std::vector<glm::vec3>& vertices,
             std::vector<glm::vec2>& uvs,
             std::vector<glm::vec3>& normals,
             std::vector<unsigned int>& indices)
{
    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier OBJ : " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string header;
        ss >> header;

        if (header == "v") {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        } else if (header == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        } else if (header == "vn") {
            glm::vec3 normal;
            ss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        } else if (header == "f") {
            std::string vertexStr;
            
            // Stockage temporaire pour les indices de cette face spécifique
            std::vector<unsigned int> f_vIndices, f_uvIndices, f_nIndices;

            while (ss >> vertexStr) {
                unsigned int vIdx = 0, vtIdx = 0, vnIdx = 0;
                
                // Format v/vt/vn
                if (sscanf(vertexStr.c_str(), "%u/%u/%u", &vIdx, &vtIdx, &vnIdx) == 3) {
                    f_vIndices.push_back(vIdx);
                    f_uvIndices.push_back(vtIdx);
                    f_nIndices.push_back(vnIdx);
                } 
                // Format v//vn
                else if (sscanf(vertexStr.c_str(), "%u//%u", &vIdx, &vnIdx) == 2) {
                    f_vIndices.push_back(vIdx);
                    f_nIndices.push_back(vnIdx);
                }
                // Format v/vt
                else if (sscanf(vertexStr.c_str(), "%u/%u", &vIdx, &vtIdx) == 2) {
                    f_vIndices.push_back(vIdx);
                    f_uvIndices.push_back(vtIdx);
                }
                // Format v
                else if (sscanf(vertexStr.c_str(), "%u", &vIdx) == 1) {
                    f_vIndices.push_back(vIdx);
                }
            }

            // Triangulation basique (Triangle Fan) pour gérer les Quads et n-gons
            if (f_vIndices.size() >= 3) {
                for (size_t i = 1; i < f_vIndices.size() - 1; ++i) {
                    vertexIndices.push_back(f_vIndices[0]);
                    vertexIndices.push_back(f_vIndices[i]);
                    vertexIndices.push_back(f_vIndices[i + 1]);

                    if (!f_uvIndices.empty() && f_uvIndices.size() == f_vIndices.size()) {
                        uvIndices.push_back(f_uvIndices[0]);
                        uvIndices.push_back(f_uvIndices[i]);
                        uvIndices.push_back(f_uvIndices[i + 1]);
                    }

                    if (!f_nIndices.empty() && f_nIndices.size() == f_vIndices.size()) {
                        normalIndices.push_back(f_nIndices[0]);
                        normalIndices.push_back(f_nIndices[i]);
                        normalIndices.push_back(f_nIndices[i + 1]);
                    }
                }
            }
        }
    }

    // Déroulage des données pour OpenGL (Indexation unique)
    for (unsigned int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vIdx = vertexIndices[i];
        
        // Attention : OBJ est indexé à partir de 1, d'où le "-1"
        vertices.push_back(temp_vertices[vIdx - 1]);
        
        // Vérification stricte via la taille au lieu de .empty()
        if (i < uvIndices.size()) {
            unsigned int vtIdx = uvIndices[i];
            uvs.push_back(temp_uvs[vtIdx - 1]);
        }
        
        if (i < normalIndices.size()) {
            unsigned int vnIdx = normalIndices[i];
            normals.push_back(temp_normals[vnIdx - 1]);
        }
        
        indices.push_back(i);
    }

    return true;
}