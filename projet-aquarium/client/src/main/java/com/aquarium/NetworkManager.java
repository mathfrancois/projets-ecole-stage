package com.aquarium;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;
import java.util.Properties;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.BlockingQueue;


public class NetworkManager {
    private Socket socket;
    private BufferedReader in;
    private PrintWriter out;

    private String id;
    private String controllerIP;
    private int controllerPort;
    private int displayTimeout;
    private String resourcesPath;

    private BlockingQueue<String> responseQueue = new LinkedBlockingQueue<>();
    private BlockingQueue<String> continuousListQueue = new LinkedBlockingQueue<>();

    // Chargement de la configuration depuis le classpath    
    public boolean loadConfig(String configFilePath) {
        Properties config = new Properties();
        System.out.println("Chargement de la configuration depuis " + configFilePath);

        // Utilisation de getResourceAsStream() pour charger le fichier depuis le classpath
        try (InputStream inputStream = getClass().getClassLoader().getResourceAsStream(configFilePath)) {
            if (inputStream == null) {
                System.err.println("Le fichier de configuration " + configFilePath + " n'a pas été trouvé dans le classpath.");
                return false;
            }
            
            // Chargement des propriétés
            config.load(inputStream);
            this.controllerIP = config.getProperty("controller-address");
            this.id = config.getProperty("id");
            this.controllerPort = Integer.parseInt(config.getProperty("controller-port"));
            this.displayTimeout = Integer.parseInt(config.getProperty("display-timeout-value"));
            this.resourcesPath = config.getProperty("resources");
            return true;

        } catch (IOException e) {
            System.err.println("Erreur lors de la lecture de la configuration : " + e.getMessage());
            return false;
        }
    }

    public AquariumView connectToController() {
        try {
            socket = new Socket(controllerIP, controllerPort);
    
            //On force ici l'encodage US_ASCII pour éviter BOM ou autres surprises
            out = new PrintWriter(new OutputStreamWriter(socket.getOutputStream(), StandardCharsets.US_ASCII), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    
            // Préparation du message avec \n
            String helloMsg = "hello in as " + id + "\n";
    
            // Envoi propre et explicite
            out.print(helloMsg);
            out.flush();  
    
            System.out.println("Envoyé : " + helloMsg.trim());
    
            String response = in.readLine();
            System.out.println("Réponse du serveur : " + response);
    
            String regex = "greeting\\s+\\S+\\s+(\\d+)x(\\d+)\\+(\\d+)\\+(\\d+)\\s+(\\d+)x(\\d+)";
            Pattern pattern = Pattern.compile(regex);
            Matcher matcher = pattern.matcher(response);
    
            if (matcher.find()) {
                double vueX = Double.parseDouble(matcher.group(1));
                double vueY = Double.parseDouble(matcher.group(2));
                double vueWidth = Double.parseDouble(matcher.group(3));
                double vueHeight = Double.parseDouble(matcher.group(4));
                double aquariumWidth = Double.parseDouble(matcher.group(5));
                double aquariumHeight = Double.parseDouble(matcher.group(6));
    
                System.out.println("Vue détectée : x=" + vueX + ", y=" + vueY + ", w=" + vueWidth + ", h=" + vueHeight +
                        " aquariumWidth=" + aquariumWidth + ", aquariumHeight=" + aquariumHeight);
    
                return new AquariumView(vueX, vueY, vueWidth, vueHeight, aquariumWidth, aquariumHeight);
            } else {
                System.err.println("Format de réponse inattendu, impossible d'extraire la vue.");
            }
    
        } catch (IOException e) {
            System.err.println("Erreur de connexion : " + e.getMessage());
        }
    
        return null;
    }
    



    public BufferedReader getInput() {
        return in;
    }

    public PrintWriter getOutput() {
        return out;
    }

    public String getResourcesPath() {
        return resourcesPath;
    }

    public int getDisplayTimeout() {
        return displayTimeout;
    }

    public String getId() {
        return id;
    }

    public void sendCommand(String command) {
        out.println(command);
        out.flush();
    }

    public void startPingLoop() {
        ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();
        executor.scheduleAtFixedRate(() -> {
            long timestamp = System.currentTimeMillis();
            out.println("ping " + timestamp);
            //System.out.println("⏱️ " + "ping" + timestamp);
        }, displayTimeout / 2, displayTimeout / 2, TimeUnit.SECONDS);
    }
    
    public void startListeningLoop() {
        new Thread(() -> {
            try {
                String line;
                while ((line = in.readLine()) != null) {
                    if (line.startsWith("pong")) {
                        // Ignore
                    } else if (line.startsWith("list")) {
                        // Si on est en mode écoute continue, on les place dans la queue dédiée
                        continuousListQueue.offer(line);
                    } else if (line.startsWith("OK") || line.startsWith("NOK") || line.startsWith("bye")) {
                        responseQueue.offer(line);
                    }
                }
            } catch (IOException e) {
                System.err.println("Erreur lors de la lecture des messages : " + e.getMessage());
            }
        }).start();
    }

    

    public boolean isConnected() {
        return socket != null && !socket.isClosed();
    }

    public String sendCommandAndWaitForResponse(String command) {
        try {
            System.out.println("Envoi de la commande : '" + command + "'");
            sendCommand(command);
            String response = responseQueue.poll(5, TimeUnit.SECONDS); // Augmentation du timeout à 5 secondes
            System.out.println("Réponse reçue : '" + response + "'");
            return response;
        } catch (InterruptedException e) {
            System.err.println("Timeout ou interruption lors de l'attente de la réponse.");
            return null;
        }
    }

    public String[] sendCommandAndWaitFor3Lists(String command) {
        String[] lists = new String[3];
        int count = 0;

        sendCommand(command); // envoie la commande une seule fois

        long timeout = System.currentTimeMillis() + 2000; // 2 secondes max en tout

        while (count < 3 && System.currentTimeMillis() < timeout) {
            try {
                // Attendre une réponse avec un délai d'attente ajusté
                String resp = responseQueue.poll(500, TimeUnit.MILLISECONDS); // Attente de 500 ms à chaque fois
                if (resp != null && resp.startsWith("list")) {
                    lists[count++] = resp;
                }
            } catch (InterruptedException e) {
                System.err.println("Erreur d'attente pour la réponse de liste.");
            }
        }

        if (count < 3) {
            System.err.println("Seulement " + count + " réponses 'list' reçues sur 3.");
        }

        return lists;
    }

    public BlockingQueue<String> getResponseQueue() {
        return responseQueue;
    }

    public BlockingQueue<String> getContinuousListQueue() {
        return continuousListQueue;
    }


}
