.PHONY: build run clean all configure

# Configure CMake (à faire une seule fois)
configure:
	cd build && cmake ..

# Compile le projet
build:
	cd build && make -j4

# Exécute le programme
run:
	cd TP1 && ./TP1

# Compile et exécute
all: build run

# Nettoie les fichiers générés
clean:
	cd build && make clean

# Nettoie tout (pour export/rebuild complet)
distclean:
	rm -rf build/*

.DEFAULT_GOAL := all
