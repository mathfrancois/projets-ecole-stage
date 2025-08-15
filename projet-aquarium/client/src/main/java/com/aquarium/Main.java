package com.aquarium;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;

import java.util.Scanner;

public class Main extends Application {
    private NetworkManager network;
    private ClientController controller;

    @Override
    public void start(Stage primaryStage) {
        network = new NetworkManager();

        if (network.loadConfig("affichage.cfg")) {
            AquariumView aquarium = network.connectToController(); // vue initialisée par le serveur
            if (aquarium == null) {
                System.err.println("Impossible de créer l'aquarium, arrêt.");
                return;
            }
            double aquariumWidth = aquarium.getPrefWidth();
            double aquariumHeight = aquarium.getPrefHeight();
            System.out.println("Aquarium : " + aquariumWidth + "x" + aquariumHeight);
            

            network.startListeningLoop();
            network.startPingLoop();

            controller = new ClientController(network, aquarium);
            controller.getFishesContinuously();
            CommandHandler handler = new CommandHandler(controller);

            // JavaFX : afficher l’aquarium
            Scene scene = new Scene(aquarium, aquariumWidth, aquariumHeight);
            primaryStage.setTitle("Aquarium Simulation");
            primaryStage.setScene(scene);
            primaryStage.show();

            // Thread à part pour le terminal
            new Thread(() -> {
                Scanner scanner = new Scanner(System.in);
                System.out.println("Client connecté. Tapez une commande (status, addFish...) ou 'log out' pour quitter.");

                while (true) {
                    System.out.print("$ ");
                    String input = scanner.nextLine().trim();
                    if (input.equalsIgnoreCase("exit")) {
                        System.exit(0); // quitte tout proprement
                        break;
                    }
                    handler.handleCommand(input);
                }
            }).start();
        } else {
            System.err.println("Échec du chargement de la configuration.");
        }
    }

    public static void main(String[] args) {
        launch(args); // démarre l'appli JavaFX
    }
}
