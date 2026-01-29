#ifndef HEIGHTMAP_LOADER_HPP
#define HEIGHTMAP_LOADER_HPP

#include <vector>

// Charge une heightmap depuis un fichier BMP
// Supporte les formats : 8 bpp (avec palette), 24 bpp, 32 bpp
// Supporte la compression RLE8
// Retourne un vecteur 2D avec les hauteurs (0.0 à 1.0)
std::vector<std::vector<float>> loadHeightmap(const char* filepath, int& width, int& height);

#endif // HEIGHTMAP_LOADER_HPP
