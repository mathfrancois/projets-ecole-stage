package com.aquarium;

import java.util.regex.Pattern;
import java.util.regex.Matcher;



public class CommandHandler {
    private final ClientController controller;

    public CommandHandler(ClientController controller) {
        this.controller = controller;
    }

    public void handleCommand(String input) {
        String[] parts = input.trim().split("\\s+");

        if (parts.length == 0) return;

        String command = parts[0].toLowerCase();

        switch (command) {
            case "status":
                System.out.println(controller.getStatus());
                break;

            case "addfish":
                handleAddFish(input);
                break;
            case "delfish":
                handleDelFish(input);
                break;
            case "startfish":
                handleStartFish(input);
                break;
            case "log":
                if (parts.length > 1 && parts[1].equalsIgnoreCase("out")) {
                    handleLogOut();
                } else {
                    System.out.println("-> NOK : commande introuvable");
                }
                break;
            default:
                System.out.println("-> NOK : commande introuvable");
                break;
        }
    }

    private void handleAddFish(String input) {
        try {
            // Exemple d'entrée : addFish julesClown at 10x50, 200x100, Horizontal
            Pattern pattern = Pattern.compile("addFish (\\w+) at (\\d+)x(\\d+), (\\d+)x(\\d+), (\\w+)");
            Matcher matcher = pattern.matcher(input);

            if (!matcher.matches()) {
                System.out.println("Utilisation : addFish Nom at XxY, WxH, MoveFunction");
                return;
            }

            String name = matcher.group(1);
            double x = Double.parseDouble(matcher.group(2));
            double y = Double.parseDouble(matcher.group(3));
            double width = Double.parseDouble(matcher.group(4));
            double height = Double.parseDouble(matcher.group(5));
            String moveFunction = matcher.group(6);

            controller.addFish(name, x, y, width, height, moveFunction);
        } catch (Exception e) {
            System.out.println("Erreur dans l'ajout du poisson : " + e.getMessage());
        }
    }

    private void handleDelFish(String input) {
        try {
            // Exemple d'entrée : delFish julesClown
            Pattern pattern = Pattern.compile("delFish (\\w+)");
            Matcher matcher = pattern.matcher(input);

            if (!matcher.matches()) {
                System.out.println("Utilisation : delFish Nom");
                return;
            }

            String name = matcher.group(1);
            controller.delFish(name);
        } catch (Exception e) {
            System.out.println("Erreur dans la suppression du poisson : " + e.getMessage());
        }
    }

    private void handleStartFish(String input) {
        try {
            // Exemple d'entrée : startFish julesClown
            Pattern pattern = Pattern.compile("startFish (\\w+)");
            Matcher matcher = pattern.matcher(input);

            if (!matcher.matches()) {
                System.out.println("Utilisation : startFish Nom");
                return;
            }

            String name = matcher.group(1);
            controller.startFish(name);
        } catch (Exception e) {
            System.out.println("Erreur dans le démarrage du poisson : " + e.getMessage());
        }
    }

    private void handleLogOut() {
        String response = controller.logOut();
        if (response != null && response.trim().equalsIgnoreCase("bye")) {
            System.out.println("-> OK : Déconnexion réussie");
            System.exit(0);
        } else {
            System.out.println("-> NOK : Erreur lors de la déconnexion");
            System.out.println("Réponse reçue : '" + response + "'");
        }
    }
}