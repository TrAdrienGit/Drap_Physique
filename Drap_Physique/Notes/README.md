# Drap Physique – Simulation de Tissu C++ / OpenGL

Une simulation de **tissu dynamique** en C++, utilisant l’intégration **Verlet**, calcul de forces de tension et collisions avec des objets géométriques (sphère, pavé, etc.).  
Entièrement réalisée avec OpenGL via un wrapper maison, shaders personnalisés, textures et matériaux. Les paramètres de simulation sont importés depuis un fichier JSON. La caméra orbite lentement pour une visualisation fluide.

---

## 📸 Démo

![Simulation du tissu sur des objets](./docs/simulation_screenshot.jpg)

*Note : insère ici une capture d’écran ou une animation montrant le drap en interaction dynamique.*

---

## ⚙️ Fonctionnalités

- **Physique du tissu**  
  - Modèle continu via Verlet (position-based dynamics)  
  - Forces de gravité, tension/mouvement élastique, amortissement, rigidité des ressorts  
  - Gestion des sous-étapes temporelles pour la stabilité de la simulation

- **Collisions**  
  - Détection et réaction sur sphères, boîtes cubiques, cônes & cylindres (activables via JSON)  
  - Anti-clipping et rebonds gérés par une rigidité paramétrable

- **Rendering OpenGL**  
  - Wrapper perso pour GLFW + GLAD + shaders  
  - Shaders vertex & fragment customisés  
  - Support de textures diffuse, speculaire, emissive  
  - Lighting basé sur modèle Phong (ambient, diffuse, specular)

- **Caméra automatisée**  
  - Orbite fluide autour de la scène selon paramètres JSON  
  - Matrices projection / view recalculées chaque frame

- **Paramétrage via JSON**  
  - Configuration complète : dimensions & résolution du tissu, physique, éléments collidants, caméra et éclairage

---

## 🧱 Structure du projet

- `src/` : code source principal  
- `shaders/` : vertex & fragment shaders  
- `textures/`, `models/` : ressources graphiques  
- `json/config.json` : fichier de configuration  
- `CMakeLists.txt` : configuration CMake

---

## 🚀 Build & run

Assurez-vous d’avoir les dépendances suivantes : GLFW, GLAD, Assimp, GLM  
(Sous Windows, on fournit les `.lib` dans `lib/`)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./Drap_Physique            # ou Drap_Physique.exe sous Windows
```

---

## ⚙️ Exemple de config.json

```{
  "activateSphere": true,
  "activateBox": true,
  "lockCorner": true,
  "tissu": {
    "sizeX": 10.8, "sizeY": 10.8,
    "resolutionX": 22, "resolutionY": 22,
    "startingHeight": 0.5, "mass": 1.0
  },
  "physics": {
    "gravity": { "amplitude": 0.04905, "directionVector": [0,0,-1] },
    "tension": { "force": 500, "maxTensionForce": 10, "damping": 200000 },
    "collision": { "antiClippingGap": 0.1, "stiffness": 50000 },
    "temporal": { "dt": 0.008333, "substeps": 20 }
  },
  ... (camera, light, screen)
}
```

---

## 🛠️ Usage

    Appuie sur ÉCHAP ou ferme la fenêtre pour quitter

    [Idée : développe ici d’autres contrôles interactifs éventuels]

---

## 🧠 Tech stack & inspirations

    C++20, OpenGL 3.3+, GLFW, GLAD, Assimp, GLM

    Simulation par Verlet/jacobi

    Inspiration de projets comme Cloth Simulation Tutorial et MyClothSimulation

---

## 📝 Fichiers clés

| Fichier                 | Description                                        |
|-------------------------|----------------------------------------------------|
| `src/tissu.h/.cpp`      | Génère le maillage du drap et gère les contraintes |
| `src/physic.h/.cpp`     | Logique Verlet + collisions                        |
| `src/simple_camera.h`   | Caméra orbitale automatisée                        |
| `src/glencapsulation.h` | Wrapper OpenGL utile                               |
| `src/shader.h`          | Compilation & gestion des shaders                  |
| `json/config.json`      | Toutes les options de simulation                   |
| `CMakeLists.txt`        | Configuration du projet CMake                      |

---

## ✅ Améliorations possibles

    Ajout d’interactions en direct (on peut bouger les objets ou la caméra à la souris)

    Self‑collision pour le tissu

    Réglages dynamiques des paramètres via GUI (IMGUI, etc.)

    Export vidéo/gif automatisé

---

## 📄 Licence

Ce projet est sous licence MIT. Voir le fichier LICENSE pour plus de détails.

---

## 👤 Auteur

Ton nom – passionné de simulation & graphisme 3D.