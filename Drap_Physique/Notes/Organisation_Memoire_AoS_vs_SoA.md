# Organisation mémoire : AoS vs SoA

## 📦 AoS (Array of Structures)

L'approche **AoS** consiste à regrouper toutes les données d'un objet (ou sommet) dans une **structure unique**, puis à stocker un tableau de ces structures.

### Exemple :

```cpp
struct Vertex {
    glm::vec3 position;
    glm::vec3 vitesse;
    glm::vec3 acceleration;
    glm::vec3 normal;
    glm::vec2 uv;
    float masse;
    bool isFixed;
};

std::vector<Vertex> vertices;
```

Chaque élément du tableau représente un **vertex complet**.

### Inconvénients :

- Mauvaise **localité mémoire** pour le CPU (cache inefficace).
- Difficile à **vectoriser automatiquement** (SIMD).
- Moins adapté à des calculs parallèles.
- Accès dispersé pour des traitements par champ (ex : toutes les positions).

---

## 🧱 SoA (Structure of Arrays)

L’approche **SoA** consiste à stocker **chaque champ séparément** dans un tableau dédié. Tous les `position` sont dans un tableau, toutes les `vitesse` dans un autre, etc.

### Exemple :

```cpp
std::vector<glm::vec3> positions;
std::vector<glm::vec3> vitesses;
std::vector<glm::vec3> accelerations;
std::vector<glm::vec3> normales;
std::vector<glm::vec2> uvs;
std::vector<float> masses;
std::vector<bool> isFixed;
```

Un **vertex est représenté par un index commun** dans tous les tableaux.

### Avantages :

✅ **Meilleure performance CPU** : données contiguës → cache efficace  
✅ **Compatibilité SIMD** : traitements vectoriels plus rapides  
✅ **Parallélisation plus facile** : OpenMP ou autres frameworks  
✅ **Accès plus rapide** sur de grandes structures (grilles, maillages)

---

## ⚙️ Pourquoi nous passons à SoA

Dans le cadre de notre projet de simulation de tissu, nous avons choisi de **remplacer le modèle AoS par une approche SoA** pour les raisons suivantes :

- Le système physique effectue de nombreux accès à des champs spécifiques (positions, vitesses, normales…)
- La simulation manipule potentiellement **des milliers de sommets**, rendant la performance critique
- Les fonctions de calcul peuvent bénéficier de la **vectorisation automatique**
- OpenMP et d'autres optimisations CPU nécessitent une séparation claire des données

---

## 🚫 Suppression de la classe `Vertex`

La conséquence directe de cette transition vers SoA est la **suppression de la classe `Vertex`**. Celle-ci n’est plus nécessaire puisque ses champs sont désormais stockés indépendamment dans des `std::vector`.

Nous utilisons désormais une classe `TissuSoA`, qui expose une interface optimisée, accédant aux champs via un index global calculé à partir des coordonnées `(x, y)`.

### Exemple :
```cpp
size_t index = tissu.getIndex(i, j);
tissu.positions[index] += tissu.vitesses[index] * dt;
```

---

## 📌 Conclusion

Le passage à **SoA** permet d'améliorer :
- L'efficacité mémoire
- Les performances du CPU
- La scalabilité du moteur physique

Cela implique une refactorisation partielle du système de simulation, mais le **gain en performances et maintenabilité** en vaut largement l'effort.
