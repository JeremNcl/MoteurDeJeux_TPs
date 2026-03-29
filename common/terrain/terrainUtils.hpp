#ifndef TERRAINUTILS_HPP
#define TERRAINUTILS_HPP

#include <vector>
#include <cmath>

namespace TerrainUtils {

    inline float getHeightAt(
        const std::vector<std::vector<float>>& heightmap, 
                        float maxHeight, 
                        float resolution, 
                        float x, 
                        float z
    ){
        int carreX = static_cast<int>(std::floor(x / resolution));
        int carreZ = static_cast<int>(std::floor(z / resolution));

        int width = heightmap[0].size();
        int height = heightmap.size(); 

        if(carreX >= 0 && carreX < width - 1 && carreZ >= 0 && carreZ < height - 1) {
            float localPosX = (x / resolution) - carreX;
            float localPosZ = (z / resolution) - carreZ;

            float h00 = heightmap[carreZ][carreX];
            float h10 = heightmap[carreZ][carreX + 1];
            float h01 = heightmap[carreZ + 1][carreX];
            float h11 = heightmap[carreZ + 1][carreX + 1];
        
            float targetHeight;
            if (localPosX + localPosZ <= 1.0f) {
                targetHeight = h00 + localPosX * (h10 - h00) + localPosZ * (h01 - h00);
            } else {
                targetHeight = h11 + (1.0f - localPosX) * (h01 - h11) + (1.0f - localPosZ) * (h10 - h11);
            }

            return targetHeight * maxHeight;
        }

        return 0.f;
    }
}

#endif