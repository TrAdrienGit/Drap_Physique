# `std::atomic` en C++ : Guide Pratique

## ✨ Qu'est-ce que `std::atomic` ?

`std::atomic<T>` est une classe fournie par la STL de C++ pour permettre des **opérations thread-safe** sur des variables partagées entre plusieurs threads **sans mutex**.

C'est une brique de base pour éviter les **conditions de course** (race conditions) dans la programmation parallèle.

---

## ⚙️ Pourquoi l'utiliser ?

Lorsqu'une variable est accédée ou modifiée par plusieurs threads **en même temps**, il faut synchroniser son accès.

Au lieu d'utiliser `std::mutex` :

```cpp
std::mutex mtx;
bool flag = true;

std::lock_guard<std::mutex> lock(mtx);
flag = false;
```

Tu peux simplement écrire :

```cpp
std::atomic<bool> flag = true;
flag.store(false);
```

---

## 🔀 Fonctions principales

| Méthode                    | Description                             |
| -------------------------- | --------------------------------------- |
| `.store(value)`            | Affecte une valeur                      |
| `.load()`                  | Récupère la valeur actuelle             |
| `.exchange(value)`         | Change la valeur et retourne l'ancienne |
| `.compare_exchange_strong` | Compare, puis change si égales          |
| `++`, `--`, `+=`, `-=`     | Opérations arithmétiques atomiques      |

---

## 📊 Exemple simple

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> counter = 0;

void count() {
    for (int i = 0; i < 1000; ++i)
        ++counter; // atomique et thread-safe
}

int main() {
    std::thread t1(count);
    std::thread t2(count);
    t1.join();
    t2.join();

    std::cout << "Total: " << counter << "\n"; // 2000 garanti
}
```

---

## 🔐 Avantages

- Pas besoin de mutex pour les opérations simples
- Plus léger et plus rapide que les verrous classiques
- Parfait pour des flags booléens ou des compteurs simples

---

## ⚠️ Précautions

- Ne protège que **la variable elle-même**
- Si tu modifies plusieurs valeurs à la fois (structs, vecteurs...) => mutex obligatoire
- Pas de rollback automatique en cas d'erreur logique (comme les transactions)

---

## 🎓 Dans ton projet

- Tu utilises `std::atomic<bool>` pour activer ou désactiver les forces (vent, gravitation, sphère...) entre la **console de commandes** (thread 1) et la **simulation physique** (thread principal)
- Tu garantis ainsi une lecture/écriture sûre sans blocage ni latence

---

## ✅ Conclusion

`std::atomic` est un outil simple, puissant et efficace pour protéger des variables partagées entre threads sans complexité. C'est parfait pour :

- Des flags `bool`
- Des compteurs `int`
- Des flags de contrôle temps réel (comme dans ta simulation)

