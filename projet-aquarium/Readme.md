# Projet Réseau Aquarium

Ce projet simule un aquarium réseau avec un serveur qui gère les poissons et un client qui affiche leur position.

## Structure du projet

- **serveur/** : Contient le code source du serveur.
- **client/** : Contient le code source du client.
- **diag/** : Contient des diagrammes d'architecture.
- **poisson/** : Contient des ressources graphiques pour les poissons.

---

## Compilation et exécution

### Prérequis

- **Serveur** :
  - GCC (compilateur C)
  - Make

- **Client** :
  - Java 11 ou supérieur
  - Maven

---

### Compilation et exécution du serveur

1. **Se déplacer dans le dossier `serveur` :**
   ```bash
   cd serveur
   ```
2. **Compiler le `serveur` :**
   ```bash
   make
   ```
3. **Exécuter le `serveur` :**
   ```bash
   ./serveur
   ```

Le serveur utilise les fichiers suivants pour initialiser les données de l'aquarium : aquarium1.txt, aquarium2.txt et aquarium3.txt

4. **Load l'aquarium :**
   ```bash
   load aquarium1.txt
   ```

---

### Compilation et exécution du client

1. **Se déplacer dans le dossier `client` :**
   ```bash
   cd client
   ```
2. **Compiler le `client` :**
   ```bash
   make compile
   ```
3. **Créer un package exécutable :**
   ```bash
   make package
   ```
4. **Exécuter le `client` :**
   ```bash
   make run
   ```

### Les commandes disponibles dans le client une fois lancé: 

- Ajouter des poissons au client (On ne peut pas ajouter un poisson dans la vue d'un autre client)
addFish <nom> at <X>x<Y>, <Largeur>x<Hauteur>, RandomWayPoint

(Les poissons disponibles sont julesClown, mathieuFish, cedricShark, cesarTuna)

- Démarrer un poisson
startFish <nom>

- Récupérer les données d'une vue
status

- Supprimer un poisson qui est actuellement dans la vue:
delFish <nom>

### Se connecter en réseau

Dans le fichier affichage.cfg qui se trouve dans client/ressources, changez la valeur de controller-address pour mettre l'adresse IP de l'appareil qui a lancé le serveur.
De plus, il faut que les clients et le serveur soient connectés au même wifi.


