# Free SGBD Project

## Description
Ce projet est un système de gestion de base de données (SGBD) libre développé dans le cadre de la deuxième année à l'ENSEIRB. Il a pour objectif de fournir une solution simple et efficace pour la gestion de flotte de camions.

## Fonctionnalités
- Création et gestion de bases de données
- Exécution de requêtes SQL
- Création d'une application web

## Utilisation de l'application

### En local

### 1. Installer PostgreSQL (si pas déjà installé)

Sur Ubuntu/Debian :
```bash
sudo apt update
sudo apt install postgresql postgresql-contrib
```

### 2. Ouvrir psql, créer un utilisateur et une base de donnée

```bash
sudo -u postgres psql
```

```sql
CREATE USER your_user WITH PASSWORD 'your_password';
CREATE DATABASE your_database OWNER your_user;
GRANT ALL PRIVILEGES ON DATABASE your_database TO your_user;
\q
```

### 3. Importer les scripts SQL
Depuis le terminal bash, dans le dossier sql/ du projet :

```bash
psql -U your_user -d your_database -h localhost -f drop.sql
psql -U your_user -d your_database -h localhost -f create.sql
psql -U your_user -d your_database -h localhost -f insert.sql
```

### 4. Installer Node.js et npm (si pas déjà fait)

```bash
sudo apt install nodejs npm
```

Puis dans le dossier racine : 

```bash
npm init -y
npm install express pg dotenv
```

### 5. Configurer le fichier .env pour une base locale

Créer un fichier .env à la racine avec :

```pgsql
PG_USER=your_user
PG_HOST=localhost
PG_DATABASE=your_database
PG_PASSWORD=your_password
PG_PORT=5432
```

### 6. Lancer le serveur:

```bash
node server.js
```

### 7. Connectez-vous au site en local sur votre navigateur:

http://localhost:8080/



## Auteurs
- **Maulard Jules**
- **Derache Cédric**
- **César Larraguetta**
- **Francois Mathieu**



