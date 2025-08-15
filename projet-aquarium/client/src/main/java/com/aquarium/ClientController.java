package com.aquarium;
import java.io.IOException;
import java.util.Locale;
import java.util.HashMap;
import java.util.Map;
import java.util.List;
import java.util.Arrays;
import java.util.Objects;
import java.util.stream.Collectors;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.ArrayList;
import java.util.Set;
import java.util.HashSet;


public class ClientController {
    private final NetworkManager networkManager;
    private final AquariumView aquariumView;

    public ClientController(NetworkManager networkManager, AquariumView aquariumView) {
        this.networkManager = networkManager;
        this.aquariumView = aquariumView;
    }

    public String getStatus(){
        boolean isConnected = networkManager.isConnected();
        String AllFishes = aquariumView.getFishStatus();
        Integer fishCount = aquariumView.getFishCount();
        if (isConnected){
            String status = "-> OK : Connecté au contrôleur, " + fishCount + " poissons trouvés.\n";
            status += AllFishes;
            return status;
        }
        else {
            return "-> Erreur : Non connecté au contrôleur.\n";
        }
    }

    public void sendListCommand(String command) {
        networkManager.sendCommand(command);
    }



    public void addFish(String name, double x, double y, double width, double height, String moveFunction) {
        Fish fish = new Fish(name, x, y, width, height, moveFunction);
        String FishName = fish.getName();

        if (!(FishName.equals("cedricShark") || FishName.equals("julesClown") || FishName.equals("mathieuFish") || FishName.equals("cesarTuna"))) {
            System.out.println("-> NOK : Nom de poisson : cedricShark, julesClown, mathieuFish ou cesarTuna");
            return;
        }

        // Construction propre de la commande
        String command = String.format(Locale.US, "addFish %s at %.2fx%.2f, %.2fx%.2f, %s",
                fish.getName(), fish.getX(), fish.getY(), fish.getWidth(), fish.getHeight(), fish.getMoveFunction());

        String response = networkManager.sendCommandAndWaitForResponse(command);

        if (response != null && response.trim().equalsIgnoreCase("OK")) {
                aquariumView.addFish(fish);
                System.out.println("-> OK : Poisson ajouté !");
        } else {
            System.out.println("-> NOK : Ajout refusé par le serveur.");
        }
    }
    

    public void delFish(String name) {
        // Construction propre de la commande pour la suppression
        String command = String.format(Locale.US, "delFish %s", name);

        // Envoyer la commande et attendre la réponse du serveur
        String response = networkManager.sendCommandAndWaitForResponse(command);

        if (response != null && response.trim().equalsIgnoreCase("OK")) {
            // Si le serveur accepte la suppression, supprimer le poisson de l'aquarium
            Fish fish = aquariumView.getFish(name);
            if (fish != null) {
                aquariumView.removeFish(fish);
            }
            System.out.println("-> OK : Poisson supprimé !");
        } else {
            // Si la réponse du serveur est différente de OK, ne pas supprimer le poisson
            System.out.println("-> NOK : Suppression refusée par le serveur.");
        }
    }

    private void addMovementToFish(String initialList, String finalList) {
        Map<String, List<Movement>> parsedMovements = MovementParser.parseFishMovements(initialList, finalList);

        for (Map.Entry<String, List<Movement>> entry : parsedMovements.entrySet()) {
            String name = entry.getKey();
            List<Movement> movements = entry.getValue();


            for (Movement move : movements) {


                boolean targetVisible = aquariumView.isFishVisible(
                    move.getTargetX(), move.getTargetY(), move.getWidth(), move.getHeight()
                );

                boolean initialVisible = aquariumView.isFishVisible(
                    move.getInitialX(), move.getInitialY(), move.getWidth(), move.getHeight()
                );

                boolean crossesView = aquariumView.doesMovementCrossView(move, move.getWidth(), move.getHeight());



                Fish fish = aquariumView.getFish(name);
                if (fish != null && fish.getHasToBeRemoved()) {
                    aquariumView.removeFish(fish);
                    fish = null;
                }
                if (fish != null) {
                    fish.addMovement(move);
                    if ( !targetVisible ){
                        fish.setHasToBeRemoved(true);
                        continue;
                    }
                    if (crossesView){
                        fish.setHasToBeRemoved(true);
                        continue;
                    }


                } else if (targetVisible || crossesView) {

                    int initialX = move.getInitialX();
                    int initialY = move.getInitialY();
                    int width = move.getWidth();
                    int height = move.getHeight();

                    Fish newFish = new Fish(name, initialX, initialY, width, height, "RandomWayPoint");
                    aquariumView.addFish(newFish);

                    newFish.addMovement(move);
                    aquariumView.animateFish(newFish);
                } else {
                }
            }
        }



    }


    public void startFish(String name) {
        Fish fish = aquariumView.getFish(name);

        if (fish == null) {
            System.out.println("Poisson introuvable : " + name);
            return;
        }

        String command = String.format(Locale.US, "startFish %s", name);
        String response = networkManager.sendCommandAndWaitForResponse(command);

        if (response != null && response.trim().equalsIgnoreCase("OK")) {
            aquariumView.animateFish(fish);
            System.out.println("-> OK : Poisson démarré !");

        } else {
            System.out.println("-> NOK : Démarrage refusé par le serveur.");
        }
    }
        


    public void getFishesContinuously() {
        networkManager.sendCommand("getFishesContinuously");

        Thread listener = new Thread(() -> {
            BlockingQueue<String> queue = networkManager.getContinuousListQueue();
            List<String> buffer = new ArrayList<>();
            while (true) {
                try {
                    String line = queue.poll(5, TimeUnit.SECONDS);
                    if (line != null && line.startsWith("list")) {
                        buffer.add(line);
                        if (buffer.size() == 2) {
                            String initialList = buffer.get(0);
                            String finalList = buffer.get(1);
                            addMovementToFish(initialList, finalList);
                            buffer.clear();
                        }
                    }
                } catch (InterruptedException e) {
                    System.err.println("Interruption lors de la récupération continue des poissons.");
                    break;
                }
            }
        });
        listener.setDaemon(true);
        listener.start();
    }

    public String logOut() {
        return networkManager.sendCommandAndWaitForResponse("log out");
    }

}
