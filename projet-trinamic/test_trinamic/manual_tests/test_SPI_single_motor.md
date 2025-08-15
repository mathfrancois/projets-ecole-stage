# Test de la partie matérielle
Avant de tester le code, il faut s'assurer que le matériel physique marche bien. Pour ce faire, on peut tester le code du catie actuel qui est reconnu fonctionnel.

# Test de la fonction implémentée en relation avec le protocole SPI
Pour tester la fonction SPI implémentée, le protocole est le suivant:
- Chercher un registre où l'on va changer et tester la valeur. Ici, ce sera TMC4671_ADC_I0_SCALE_OFFSET, car sa valeur par défaut est différente de 0 (256).
- Tester la lecture de ce registre pour vérifier que la valeur soit bien celle par défaut
- Mettre cette valeur à 250
- Lire la valeur du registre et vérifier qu'elle est bien à 250
- Remettre la valeur du registre à 256

# Remarques:
## Vérifier les valeurs de la TMC à l'aide de la carte mère
Pour vérifier les valeurs écrites et lues, on utilise un usbserial.
Pour l'utiliser, on doit éxecuter la commande OpenBSD cu (https://github.com/tobhe/opencu).
Cela permet d'obtenir un terminal qui lit l'entrée du port série /dev/ttyUSB1 (sur Linux),
similaire à ce que ferait l'Arduino IDE, mais dans un terminal (et sans avoir à installer Arduino IDE).

## Compatibilité des fichiers C/C++
Les fichiers de la tmcAPI sont en .c et notre programme est en .cpp.
Ainsi, nous avons des problèmes de "name mangling", autrement dit, le compilateur, pour des programmes cpp, renomme les symboles utilisés pour l'édition des liens (pour éviter les problèmes de surcharge).
Or, comme les fichiers .c ne subissent pas ces transformations, nous avons des problèmes d'édition de lien.
Pour résoudre ce problème, nous pouvons soit renommer les fichiers .c en .cpp, soit déclarer la signature de cette fonction ainsi que les include de fichier C dans un bloc "extern C".
