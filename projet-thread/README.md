# libthread — Bibliothèque de Threads Utilisateur en C

## Description

`libthread` est une bibliothèque de threads en espace utilisateur écrite en C. Elle repose sur les primitives bas niveau (`ucontext.h`) et fournit un système de threading léger, coopératif et préemptif, sans utiliser `pthread`.

Une extension (`libthread_ext`) ajoute la gestion des priorités, l’ordonnancement à files multiples, et le vieillissement (`aging`).

---

## Structure du projet

```
.
├── src/                 # Code source de la bibliothèque
│   ├── thread.c         # Bibliothèque de base
│   └── thread_ext.c     # Extension avec gestion des priorités
├── test/                # Tests unitaires et de performance pour libthread
├── benchmark/           # Scripts de benchmark
├── build/               # Dossier de compilation
├── install/             # Installation de la lib et des binaires
├── Makefile             # Compilation & automatisation
└── README.md           
```

---

## Compilation

### Compiler la bibliothèque de base + les tests :

```bash
make
```

### Compiler la version étendue avec gestion des priorités :

```bash
make thread_ext
```

> Le système crée automatiquement les répertoires `build/`, `build/test/`, etc.

---

## Exécution des tests

### 1. Lancer tous les tests de la bibliothèque `libthread` :

```bash
make check
```

### 2. Lancer les tests avec `valgrind` (vérification mémoire) :

```bash
make valgrind
```

### 3. Lancer les tests pour `libthread_ext` :

```bash
make check_ext
```

### 4. Lancer les tests `libthread_ext` avec `valgrind` :

```bash
make valgrind_ext
```

---

## Options de compilation

Active ou désactive certaines fonctionnalités lors du `make` :

| Variable             | Valeur | Description                          |
|----------------------|--------|--------------------------------------|
| ENABLE_PREEMPTION    | 0 / 1  | Active la préemption (`par défaut : 1`) |
| ENABLE_LOGGING       | 0 / 1  | Affiche les logs internes            |
| ENABLE_NDEBUG        | 0 / 1  | Supprime les assertions (`NDEBUG`)   |


---

## Installation

Pour copier la bibliothèque et les exécutables dans `install/` :

```bash
make install
```

- `install/lib/libthread.a`
- `install/bin/` contient les exécutables de test

---

## Benchmarks

### Lancer tous les benchmarks :

```bash
make graphs
```

### Lancer un benchmark pour un test spécifique (exemple : test 23) :

```bash
make graphs_23
```

---

## Liste des tests

**Tests simples (sans paramètres) :**

- `01-main` : Création d’un thread
- `02-switch` : Changement de contexte
- `03-equity` : Répartition équitable
- `11-join`, `12-join-main`, `13-join-switch` : Synchronisation avec `join`
- `63-mutex-equity`, `64-mutex-join` : Synchronisation avec `mutex`

**Tests avec un paramètre (exécutés avec `$(PARAM1)` = 10) :**

- `21-create-many`, `22-create-many-recursive`, `23-create-many-once`
- `51-fibonacci` : Calcul récursif
- `71-preemption` : Test de préemption
- `61-mutex`, `62-mutex` : Mutex de base

**Tests avec deux paramètres (exécutés avec `$(PARAM1)=10`, `$(PARAM2)=10`) :**

- `31-switch-many`, `32-switch-many-join`, `33-switch-many-cascade`

**Tests de `libthread_ext` (priorité) :**

- `priority-test`, `priority_test_basic`, `priority_test_starvation`, `priority_test_stress`
- `priority_test_efficacity` (avec paramètre)

---

## Nettoyage

Pour supprimer tous les fichiers compilés :

```bash
make clean
```

---

## Auteurs

Jules Maulard, Cédric Dérache, César Larragueta, Mathieu Francois

---

