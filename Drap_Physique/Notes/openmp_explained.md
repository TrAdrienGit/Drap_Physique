# OpenMP : Fonctionnement Technique et Application en Simulation Physique

## 👁️ Qu’est-ce qu’OpenMP ?

**OpenMP (Open Multi-Processing)** est une **API de parallélisation pour CPU multi-cœurs**. Elle permet d’exécuter des boucles (ou blocs de code) **en parallèle** sur plusieurs threads **sans avoir à gérer manuellement les threads**.

---

## ⚙️ Comment ça fonctionne ?

### 1. Compilation parallèle

Quand tu ajoutes `#pragma omp parallel for` :

- Le compilateur transforme la boucle en **bloc multithread**.
- OpenMP crée un **pool de threads** (souvent 1 par cœur).
- Chaque thread exécute **une partie des itérations** de la boucle.

### 2. Exécution sur plusieurs cœurs

```cpp
#pragma omp parallel for
for (int i = 0; i < 1000; ++i) {
    data[i] += 1.0f;
}
```

Sur un CPU 4 cœurs :

- Thread 0 → `i = 0 à 249`
- Thread 1 → `i = 250 à 499`
- Thread 2 → `i = 500 à 749`
- Thread 3 → `i = 750 à 999`

Tout s’exécute en **parallèle** → boucle jusqu'à **4x plus rapide**.

### 3. Synchronisation automatique

Tous les threads terminent leur exécution, puis une **barrière implicite** est insérée à la fin de la boucle.

### 4. Sécurité mémoire

OpenMP ne protège rien par défaut. Tu dois :

- éviter d’écrire sur les mêmes variables entre threads
- travailler sur des indices `i` uniques pour `data[i]`

---

## 🔦 Pourquoi SoA + OpenMP = 💥 Performance

| Structure AoS                       | SoA                                                   |
| ----------------------------------- | ----------------------------------------------------- |
| `vector<Vertex>` → accès éparpillés | `vector<positions>, vector<acc>` → accès **contigus** |
| Mauvais pour le cache CPU           | Très bon pour le cache CPU                            |
| Parallélisation plus difficile      | **Parallélisation facile** (par index)                |

---

## 🔮 Directives utiles OpenMP

| Directive                  | Description               |
| -------------------------- | ------------------------- |
| `#pragma omp parallel for` | boucle parallèle          |
| `#pragma omp critical`     | section exclusive         |
| `#pragma omp atomic`       | opération atomique        |
| `omp_get_thread_num()`     | ID du thread courant      |
| `omp_set_num_threads(n)`   | fixe le nombre de threads |
| `omp_get_num_threads()`    | nombre total de threads   |

---

## 🔌 Exemple concret dans ta simulation

```cpp
#pragma omp parallel for
for (int i = 0; i < (int)tissu.positions.size(); ++i) {
    if (!tissu.isFixed[i])
        tissu.accelerations[i] += gravity;
}
```

- Pas de dépendance entre `i`
- Accès contigu (SoA) → performant
- Chaque thread écrit dans sa cellule

---

## 🔧 Diagramme simplifié

```txt
Thread 0:  | ##########          |
Thread 1:  |          ########## |
             ↑ chaque thread traite une portion du tableau

→ Résultat : boucle entière 2x à 10x plus rapide selon CPU
```

---

## ✅ Conclusion

- OpenMP transforme ton code mono-cœur en moteur parallèle puissant
- SoA permet d’accélérer encore plus en accédant mémoire efficacement
- Bien utilisé, OpenMP → **simulation fluide, scalable et rapide**

Souhaite-tu une version avec visualisation graphique ou un chronométrage FPS à l'écran ?

