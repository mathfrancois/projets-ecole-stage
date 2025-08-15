package com.aquarium;

import javafx.animation.TranslateTransition;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.Pane;
import javafx.util.Duration;
import javafx.animation.PauseTransition;
import java.util.HashMap;
import java.util.Map;
import javafx.application.Platform;
import javafx.animation.KeyFrame;
import javafx.animation.KeyValue;
import javafx.animation.Timeline;

public class AquariumView extends Pane {
    private final Map<Fish, ImageView> fishMap = new HashMap<>();
    private double x, y;
    private double ViewWidth, ViewHeight;
    private double AquariumWidth;
    private double AquariumHeight;


    public AquariumView(double x, double y, double width, double height, double aquariumWidth, double aquariumHeight) {
        this.x = x;
        this.y = y;
        this.ViewWidth = width;
        this.ViewHeight = height;
        this.AquariumWidth = aquariumWidth;
        this.AquariumHeight = aquariumHeight;

        setPrefSize(width, height);
        setMinSize(width, height);
        setMaxSize(width, height);

        ImageView background = new ImageView(new Image(getClass().getResource("/background/background.jpeg").toString()));
        background.setFitWidth(width);
        background.setFitHeight(height);
        background.setPreserveRatio(false);
        background.setSmooth(true);
        getChildren().add(background);
    }

    public Fish getFish(String name) {
        for (Fish fish : fishMap.keySet()) {
            if (fish.getName().equals(name)) {
                return fish;
            }
        }
        return null;
    }

    public void addFish(Fish fish) {
        try {
            ImageView fishView = new ImageView(new Image(getClass().getResource("/fishes/" + fish.getName() + ".png").toString()));
            fishView.setFitWidth(fish.getWidth());
            fishView.setFitHeight(fish.getHeight());
            
            double x_aqua = (fish.getX() /100) * this.AquariumWidth;
            double y_aqua = (fish.getY() /100) * this.AquariumHeight;

            //  Coordonnées locales
            double pixelX = x_aqua - this.x;
            double pixelY = y_aqua - this.y;

            fishMap.put(fish, fishView); // Mettre d'abord dans la map
            Platform.runLater(() -> {
                // Applique les coordonnées en pixels dans l'interface graphique
                fishView.setLayoutX(pixelX);
                fishView.setLayoutY(pixelY);
                getChildren().add(fishView);
            });

        } catch (Exception e) {
            System.err.println("Erreur lors du chargement du poisson " + fish.getName());
            e.printStackTrace();
        }
    }

    public void animateFish(Fish fish) {
        Movement move = fish.pollMovement();
        if (move == null) {
            // Attendre 0.1 seconde et réessayer
            PauseTransition wait = new PauseTransition(Duration.seconds(0.1));
            wait.setOnFinished(event -> animateFish(fish));
            wait.play();
            return;
        }

        double startX = fish.getX();
        double startY = fish.getY();
        double moveStartX = move.getInitialX();
        double moveStartY = move.getInitialY();

        if (startX != moveStartX || startY != moveStartY) {
            System.out.println("Erreur de positionnement du poisson " + fish.getName() + " : " +
                            "Position actuelle (" + startX + ", " + startY + ") " +
                            "différente de la position de départ du mouvement (" + moveStartX + ", " + moveStartY + ")");
            return;
        }

        double targetX = move.getTargetX();
        double targetY = move.getTargetY();

        ImageView view = fishMap.get(fish);
        if (view == null) return;

        // Calculs de position dans la scène
        double sceneStartX = (startX / 100.0) * this.AquariumWidth - this.x;
        double sceneStartY = (startY / 100.0) * this.AquariumHeight - this.y;
        double sceneTargetX = (targetX / 100.0) * this.AquariumWidth - this.x;
        double sceneTargetY = (targetY / 100.0) * this.AquariumHeight - this.y;

        // Final variables pour la lambda
        final ImageView finalView = view;
        final double finalSceneStartX = sceneStartX;
        final double finalSceneStartY = sceneStartY;
        final double finalSceneTargetX = sceneTargetX;
        final double finalSceneTargetY = sceneTargetY;
        final Movement finalMove = move;
        final double finalTargetX = targetX;
        final double finalTargetY = targetY;
        final Fish finalFish = fish;

        Platform.runLater(() -> {
            finalView.setLayoutX(finalSceneStartX);
            finalView.setLayoutY(finalSceneStartY);

            Timeline timeline = new Timeline(
                new KeyFrame(Duration.seconds(finalMove.getDurationInSeconds()),
                    new KeyValue(finalView.layoutXProperty(), finalSceneTargetX),
                    new KeyValue(finalView.layoutYProperty(), finalSceneTargetY)
                )
            );

            timeline.setOnFinished(e -> {
                finalFish.setX(finalTargetX);
                finalFish.setY(finalTargetY);

                PauseTransition pause = new PauseTransition(Duration.millis(50));
                pause.setOnFinished(p -> {
                    ImageView updatedView = fishMap.get(finalFish);
                    if (updatedView == null) return;

                    double layoutX = updatedView.getLayoutX();
                    double layoutY = updatedView.getLayoutY();

                    boolean removed = finalFish.getHasToBeRemoved();

                    animateFish(finalFish);
                });
                pause.play();
            });

            timeline.play();
        });
    }

    public boolean isFishVisible(double fishX, double fishY, double fishWidthPixels, double fishHeightPixels) {
        // Vue en coordonnées normalisées (en %)
        double viewXNorm = (x / AquariumWidth) * 100.0;
        double viewYNorm = (y / AquariumHeight) * 100.0;
        double viewWidthNorm = (ViewWidth / AquariumWidth) * 100.0;
        double viewHeightNorm = (ViewHeight / AquariumHeight) * 100.0;

        // Dimensions du poisson converties en %
        double fishWidthNorm = (fishWidthPixels / AquariumWidth) * 100.0;
        double fishHeightNorm = (fishHeightPixels / AquariumHeight) * 100.0;

        // Rectangle intersection
        boolean horizontalOverlap = fishX + fishWidthNorm >= viewXNorm &&
                                    fishX <= viewXNorm + viewWidthNorm;

        boolean verticalOverlap = fishY + fishHeightNorm >= viewYNorm &&
                                fishY <= viewYNorm + viewHeightNorm;

        return horizontalOverlap && verticalOverlap;
    }

    public void removeFish(Fish fish) {
        Platform.runLater(() -> {
            ImageView fishView = fishMap.remove(fish);
            if (fishView != null) {
                getChildren().remove(fishView);
            }
        });
    }

    public boolean contains(Fish fish) {
        return isFishVisible(fish.getX(), fish.getY(), fish.getWidth(), fish.getHeight());
    }

    public Integer getFishCount() {
        return fishMap.size();
    }

    public Map<Fish, ImageView> getFishMap() {
        return fishMap;
    }

    public String getFishStatus() {
        StringBuilder status = new StringBuilder();
        for (Map.Entry<Fish, ImageView> entry : fishMap.entrySet()) {
            Fish fish = entry.getKey();
            ImageView fishView = entry.getValue();
            status.append("Fish ").append(fish.getName()).append(" at ")
                  .append(fish.getX()).append("x").append(fish.getY()).append(" , ")
                  .append(fish.getWidth()).append("x").append(fish.getHeight()).append("\n");
        }
        return status.toString();
    }

    public void printFishStatus() {
        for (Map.Entry<Fish, ImageView> entry : fishMap.entrySet()) {
            Fish fish = entry.getKey();
            System.out.println("Fish " + fish.getName() + " at " + fish.getX() + "x" + fish.getY() +
                               " , " + fish.getWidth() + "x" + fish.getHeight());
        }
    }


    private boolean pointInRect(double px, double py, double rx1, double ry1, double rx2, double ry2) {
        return px >= rx1 && px <= rx2 && py >= ry1 && py <= ry2;
    }

    // Test si un segment coupe un rectangle
    private boolean lineIntersectsRect(double x1, double y1, double x2, double y2,
                                    double rx1, double ry1, double rx2, double ry2) {
        return lineIntersectsLine(x1, y1, x2, y2, rx1, ry1, rx1, ry2) || // gauche
            lineIntersectsLine(x1, y1, x2, y2, rx2, ry1, rx2, ry2) || // droite
            lineIntersectsLine(x1, y1, x2, y2, rx1, ry1, rx2, ry1) || // haut
            lineIntersectsLine(x1, y1, x2, y2, rx1, ry2, rx2, ry2);   // bas
    }

    // Algorithme de détection d'intersection de segments
    private boolean lineIntersectsLine(double x1, double y1, double x2, double y2,
                                    double x3, double y3, double x4, double y4) {
        double denom = (y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1);
        if (denom == 0.0) return false; // parallèles

        double ua = ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3)) / denom;
        double ub = ((x2 - x1)*(y1 - y3) - (y2 - y1)*(x1 - x3)) / denom;

        return (ua >= 0.0 && ua <= 1.0) && (ub >= 0.0 && ub <= 1.0);
    }


    public boolean doesMovementCrossView(Movement move, double fishWidth, double fishHeight) {
        double x1 = move.getInitialX();
        double y1 = move.getInitialY();
        double x2 = move.getTargetX();
        double y2 = move.getTargetY();

        // Coordonnées normalisées de la vue
        double viewX1 = (x / AquariumWidth) * 100.0;
        double viewY1 = (y / AquariumHeight) * 100.0;
        double viewX2 = viewX1 + (ViewWidth / AquariumWidth) * 100.0;
        double viewY2 = viewY1 + (ViewHeight / AquariumHeight) * 100.0;

        // Taille du poisson en pourcentage
        double fishWPercent = (fishWidth / AquariumWidth) * 100.0;
        double fishHPercent = (fishHeight / AquariumHeight) * 100.0;

        // Agrandir la vue pour inclure le poisson complet
        viewX1 -= fishWPercent / 2.0;
        viewY1 -= fishHPercent / 2.0;
        viewX2 += fishWPercent / 2.0;
        viewY2 += fishHPercent / 2.0;

        // Vérifier si le mouvement (avec le corps du poisson) traverse cette vue élargie
        return pointInRect(x1, y1, viewX1, viewY1, viewX2, viewY2) ||
            pointInRect(x2, y2, viewX1, viewY1, viewX2, viewY2) ||
            lineIntersectsRect(x1, y1, x2, y2, viewX1, viewY1, viewX2, viewY2);
    }



}
