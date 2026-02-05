#include "heightmap_loader.hpp"
#include <stdio.h>
#include <stdlib.h>

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
    int compression = *(int*)&(header[0x1E]);  // Compression type
    
    // Vérifier si la hauteur est négative (BMP top-down)
    bool isTopDown = false;
    if (height < 0) {
        height = -height;
        isTopDown = true;
    }
    
    // Corrections pour les BMP malformés
    int bytesPerPixel = bpp / 8;
    // Calculer le padding : chaque ligne BMP doit être un multiple de 4 octets
    int rowSize = ((width * bpp + 31) / 32) * 4;
    if (imageSize == 0) imageSize = rowSize * height;
    if (dataPos == 0) dataPos = 54;
    
    printf("Heightmap size: %d x %d, %d bpp, compression: %d, dataPos: %d, rowSize: %d, topDown: %s\n", 
           width, height, bpp, compression, dataPos, rowSize, isTopDown ? "yes" : "no");
    
    // Lire la palette de couleurs si nécessaire (pour 8 bpp)
    unsigned char* palette = nullptr;
    int paletteSize = 0;
    if (bpp == 8) {
        // Palette de 256 couleurs, 4 octets par couleur (BGRA)
        paletteSize = 256 * 4;
        palette = new unsigned char[paletteSize];
        fseek(file, 54, SEEK_SET);  // La palette commence après l'en-tête de 54 octets
        fread(palette, 1, paletteSize, file);
    }
    
    // Lire les données d'image
    unsigned char* compressedData = new unsigned char[imageSize];
    fseek(file, dataPos, SEEK_SET);
    fread(compressedData, 1, imageSize, file);
    fclose(file);
    
    // Allouer un buffer pour les données décompressées
    unsigned char* data = new unsigned char[width * height];
    
    // Décompresser RLE8 si nécessaire
    if (compression == 1 && bpp == 8) {  // BI_RLE8
        printf("Decompressing RLE8...\n");
        int x = 0, y = height - 1;  // RLE8 commence en bas à gauche
        int i = 0;
        
        while (i < imageSize && y >= 0) {
            unsigned char count = compressedData[i++];
            unsigned char value = compressedData[i++];
            
            if (count == 0) {  // Code d'échappement
                if (value == 0) {  // Fin de ligne
                    x = 0;
                    y--;
                } else if (value == 1) {  // Fin du bitmap
                    break;
                } else if (value == 2) {  // Delta
                    x += compressedData[i++];
                    y -= compressedData[i++];
                } else {  // Mode absolu
                    for (int j = 0; j < value; j++) {
                        if (y >= 0 && y < height && x < width) {
                            data[y * width + x] = compressedData[i++];
                        }
                        x++;
                    }
                    // Alignement sur mot pair
                    if (value % 2 == 1) i++;
                }
            } else {  // Mode encodé
                for (int j = 0; j < count; j++) {
                    if (y >= 0 && y < height && x < width) {
                        data[y * width + x] = value;
                    }
                    x++;
                }
            }
        }
        delete[] compressedData;
    } else {
        // Pas de compression, copier directement
        delete[] data;
        data = compressedData;
    }
    
    // Convertir les données en heightmap (normaliser entre 0 et 1)
    std::vector<std::vector<float>> heightmap(height, std::vector<float>(width));
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            unsigned char pixelValue;
            
            if (compression == 1 && bpp == 8) {
                // Données déjà décompressées en RLE8
                int pixelIndex = y * width + x;
                pixelValue = data[pixelIndex];
            } else {
                // BMP non compressé : tenir compte du padding et de l'ordre bottom-up
                int fileY = isTopDown ? y : (height - 1 - y);
                int pixelIndex = fileY * rowSize + x * bytesPerPixel;
                
                if (bpp == 24 || bpp == 32) {
                    // Pour RGB/RGBA, prendre un des canaux (ils sont identiques en grayscale)
                    pixelValue = data[pixelIndex];  // Canal B (BGR)
                } else if (bpp == 8) {
                    // Pour 8 bits, utiliser directement la valeur
                    pixelValue = data[pixelIndex];
                } else {
                    pixelValue = 0;
                }
            }
            
            // Normaliser entre 0 et 1
            heightmap[y][x] = (float)pixelValue / 255.0f;
        }
    }
    
    if (palette != nullptr) {
        delete[] palette;
    }
    delete[] data;
    
    return heightmap;
}
