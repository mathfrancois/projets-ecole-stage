package com.aquarium;

import java.util.*;
import java.util.regex.*;

public class MovementParser {

    private static final Pattern pattern = Pattern.compile("\\[(\\w+) at (\\d+)x(\\d+),(\\d+)x(\\d+),(\\d+)\\]");

    public static void printMovements(Map<String, List<Movement>> movements) {
        for (Map.Entry<String, List<Movement>> entry : movements.entrySet()) {
            String fishName = entry.getKey();
            List<Movement> movementList = entry.getValue();
            System.out.println("Poisson: " + fishName);
            for (Movement movement : movementList) {
                Movement.printMovement(movement);
            }
        }
    }

    public static Map<String, List<Movement>> parseFishMovements(String initialList, String finalList) {
        Map<String, List<Movement>> result = new HashMap<>();
        
        // Parser les deux listes
        Map<String, Movement> initialPositions = parsePositions(initialList);
        Map<String, Movement> finalPositions = parsePositions(finalList);

        // Associer les positions initiales et finales pour chaque poisson
        for (String fishName : initialPositions.keySet()) {
            Movement initialMovement = initialPositions.get(fishName);
            Movement finalMovement = finalPositions.get(fishName);

            if (finalMovement != null) {
                // Créer un objet Movement avec les positions initiales et finales
                Movement movement = new Movement(
                        initialMovement.getInitialX(), initialMovement.getInitialY(),
                        finalMovement.getTargetX(), finalMovement.getTargetY(),
                        initialMovement.getWidth(), initialMovement.getHeight(),
                        finalMovement.getDurationInSeconds()
                );

                // Ajouter le mouvement à la map associée au nom du poisson
                result.computeIfAbsent(fishName, k -> new ArrayList<>()).add(movement);
            }
        }

        return result;
    }

    // Méthode pour extraire les positions d'une liste
    private static Map<String, Movement> parsePositions(String list) {
        Map<String, Movement> positions = new HashMap<>();
        Matcher matcher = pattern.matcher(list);

        while (matcher.find()) {
            String name = matcher.group(1);
            int x = Integer.parseInt(matcher.group(2)); // Position X
            int y = Integer.parseInt(matcher.group(3)); // Position Y
            int width = Integer.parseInt(matcher.group(4)); // Largeur
            int height = Integer.parseInt(matcher.group(5)); // Hauteur
            int duration = Integer.parseInt(matcher.group(6)); // Durée du mouvement (0 ou autre)

            // Créer un objet Movement représentant la position initiale ou finale
            Movement move = new Movement(x, y, x, y, width, height, duration);

            // Stocker dans la map par nom de poisson
            positions.put(name, move);
        }

        return positions;
    }
}
