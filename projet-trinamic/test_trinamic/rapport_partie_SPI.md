# Architecture du robot
Le robot est constitué d'une carte électronique principale que l'on appelera par la suite carte mère, connectée à 4 moteurs, 1 dribbleur, 1 kicker, 1 carte électronique reliée à l'antenne radio, qui est un composant séparé des autres, acheté sur le marché.

La dernière version du robot incluait 4 moteurs dits "programmables", exécutant du code écrit par l'utilisateur sur la carte moteur associé.
Les nouveaux moteurs que ce projet met en place sont "paramétrables", et ne nécessitent qu'une configuration des paramètres initiaux, le reste des fonctionnalités étants assurées par l'API partagée par le fournisseur.
Cette configuration initiale nécessaire est assurée par un protocole de communication avec les cartes électroniques.

# Protocole de communication
Concernant les protocoles de communication utilisés entre la carte mère et les moteurs, le dribbleur et le kicker, nous utilisons le protocole ##SPI##.

## Le protocole SPI
Le protocole SPI est un protocole de communication synchrone en série, full duplex, de type maître-esclave.

### Relation de type maître-esclave
Ce type de relation signifie qu'un des systèmes commnunicant "controle" les autres en ayant le droit de débutter et d'arrêter les échanges. Au début d'un échange, le `maître` est le seul à pouvoir envoyer des données, et il peut autoriser l'`esclave` à lui répondre.

### Synchrone
Le terme de communication synchrone signifie qu'une horloge est utilisée par le maître, et transmise via un bus de données aux esclaves.
Cela permet ainsi de synchroniser les transmissions, mais nécessite un bus de données supplémentaire.
De plus, cela permet de ne pas avoir à gérer des horloges séparées pour chaque système communicant, représentant un gain de temps significatif car il n'y a pas besoin de les synchroniser tous les pas de temps.

### Full duplex
Une relation en full duplex signifie qu'elle possède 2 bus de données permettant aux 2 systèmes communicant de partager des données en même temps.
Cela nécessite donc plus de bus que d'autres modes de communications, mais permet de ne pas avoir à gérer le multiplexage.

### Série
Une communication en série s'oppose à une communication en parallèle.
La communication en parallèle utilise de nombreux bus de données pour envoyer et recevoir les données, permettant de communiquer plus rapidement. En revanche, cela nécessite donc plus de matériel, et de place sur le système communicant.
La communication en série utilise un seul bus pour envoyer ou recevoir les données, étant ainsi plus lent que le parallèle.
Dans les systèmes actuels, la plupart utilise des communications en série, car les bus prennent trop de place sur la carte électronique.

## TMC-API
Afin de communiquer correctement avec les moteurs TMC, une API a été partagée par les fournisseurs.
Pour être la plus générique possible, et s'adapter à tous types de systèmes, l'API dispose d'une fonction abstraite utilisée dans de nombreuses fonctions.
Cette fonction abstraite permet la lecture et l'envoie de données au matériel.

## MBED
Pour implémenter la fonction abstraite de la TMC-API, nous avons utilisé les fonctions et définitions fournies par mbed, un Real-Time Operating System (RTOS).
Nous avons fait ce choix, car nous souhaitions profiter de la sécurité apportée par ce genre de logiciels pour éviter d'abîmer, ou de casser du matériel électronique coûteux.