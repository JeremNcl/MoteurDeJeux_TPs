# TP1 - Moteur de Jeux

## Démarrage rapide

### Configuration initiale (une seule fois)
```bash
make configure
```
Génère les fichiers de construction CMake dans le dossier `build/`.

### Compilation et exécution
```bash
make all
```
Compile le projet et exécute le programme directement.

---

## Commandes disponibles

### `make configure`
- **Description** : Configure CMake et génère les Makefile
- **Quand l'utiliser** : Une seule fois au début, ou après des changements importants dans `CMakeLists.txt`
- **Commande équivalente** : `cd build && cmake ..`

### `make build`
- **Description** : Compile le projet
- **Quand l'utiliser** : Après avoir modifié le code source
- **Utilise** : 4 threads parallèles (`-j4`)
- **Commande équivalente** : `cd build && make -j4`

### `make run`
- **Description** : Lance le programme exécutable
- **Quand l'utiliser** : Après une compilation réussie
- **Commande équivalente** : `cd TP1 && ./TP1`

### `make all` (défaut)
- **Description** : Compile **ET** exécute le programme
- **Quand l'utiliser** : Workflow de développement normal
- **Équivalent à** : `make build && make run`

### `make clean`
- **Description** : Supprime les fichiers objet et exécutables (conserve la configuration CMake)
- **Quand l'utiliser** : Pour nettoyer avant un build complet sans reconfigurer
- **Commande équivalente** : `cd build && make clean`

### `make distclean`
- **Description** : Supprime tout le contenu du dossier `build/`
- **Quand l'utiliser** : Pour préparer le projet à une exportation ou un rebuild complet depuis zéro
- **Commande équivalente** : `rm -rf build/*`

---

## Structure du projet

```
HAI819I_TP1/
├── CMakeLists.txt          # Configuration CMake principal
├── Makefile                # Script de compilation personnalisé
├── README.md               # Ce fichier
├── build/                  # Dossier de compilation (généré par CMake)
├── TP1/                    # Code source et shaders
│   ├── TP1.cpp
│   ├── vertex_shader.glsl
│   ├── fragment_shader.glsl
│   └── textures/
│       ├── grass.bmp
│       ├── rock.bmp
│       └── snowrocks.bmp
├── common/                 # Code utilitaire partagé
└── external/               # Bibliothèques externes (GLFW, GLEW, GLM, etc.)
```

---

## Contrôles de l'application

### Général
- **ESC** : Quitter l'application

### Modes de caméra
- **C** : Basculer entre les modes caméra (Fixe → Libre → Orbitale → Libre → ...)
- **F** : Retour direct au mode caméra fixe

### Mode Caméra Libre
- **Z** : Avancer
- **S** : Reculer
- **Q** : Déplacement à gauche
- **D** : Déplacement à droite
- **Espace** : Monter
- **Shift gauche** : Descendre
- **Souris** : Orientation de la vue

### Mode Caméra Orbitale
- **Flèche Haut** : Augmenter la vitesse de rotation
- **Flèche Bas** : Diminuer la vitesse de rotation

### Modification du terrain
- **+** (ou **+** pavé numérique) : Augmenter la résolution du terrain (mesh plus détaillé)
- **-** (ou **-** pavé numérique) : Diminuer la résolution du terrain (mesh moins détaillé)

---

## Workflow de développement typique

**Premier démarrage :**
```bash
make all          # Configure, compile et exécute
```

**Modifications ultérieures :**
```bash
make all          # Recompile et réexécute automatiquement
```

**Si vous modifiez `CMakeLists.txt` :**
```bash
make configure    # Reconfigure CMake
make build        # Recompile
make run          # Exécute
```

**Avant de commit ou exporter le projet :**
```bash
make distclean    # Nettoie tout (supprime build/)
make configure    # Reconfigure
make all          # Build complet et test
```

---

## Options avancées

### Compiler avec une version de débogage
```bash
cd build && cmake -DCMAKE_BUILD_TYPE=Debug ..
make build
```

### Compiler avec optimisations
```bash
cd build && cmake -DCMAKE_BUILD_TYPE=Release ..
make build
```

---

## Dépannage

**Q : Je reçois une erreur CMake**
```bash
make distclean
make configure
make build
```

**Q : Le programme ne compile pas après des modifications**
```bash
make clean      # Efface les objets compilés
make build      # Recompile depuis zéro
```

**Q : Les shaders ou textures ne se chargent pas**
- Assurez-vous d'exécuter depuis la racine du projet avec `make run`
- Vérifiez que les fichiers existent dans `TP1/textures/` et `TP1/*.glsl`

---

## Notes

- Tous les chemins de fichiers dans le code doivent être **relatifs** au dossier `TP1/`
- Les textures doivent être en format **BMP 24-bit** (voir `loadBMP_custom()` dans `common/texture.cpp`)
- Utilisez `make all` comme commande par défaut pour un workflow fluide

