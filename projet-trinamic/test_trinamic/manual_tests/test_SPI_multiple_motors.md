# Remarque
Ce test est à effectuer après avoir testé au préalable la partie matérielle et, de préférence, après avoir suivi le protocole de test d'un seul moteur.

# Contenu du test
Ce test permet de tester le contrôle des 4 roues lorsqu'elles sont intégrées au robot.
Les étapes sont donc les suivantes: 
- Envoyer des vitesses bien distinctes aux 4 moteurs via le protocole SPI, en changeant la valeur du registre de vitesse
- Mesurer pour chaque moteur sa vitesse actuelle en lisant la valeur du registre de vitesse
- Vérifier que les mesures sont cohérentes en terme de valeurs
- Vérifier que les vitesses des moteurs correspondent bien aux moteurs que l'on souhaitait
- Utiliser le code de génération de graphe pour obtenir des informations sur l'évolution de la vitesse des moteurs en fonction des ordres de vitesse

