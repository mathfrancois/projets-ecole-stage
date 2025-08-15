package com.aquarium;

public class Movement {
    private int initialX, initialY;
    private int targetX, targetY;
    private int durationInSeconds;
    private int width, height;

    public Movement(int initialX, int initialY, int targetX, int targetY, int width, int height, int duration) {
        this.initialX = initialX;
        this.initialY = initialY;
        this.targetX = targetX;
        this.targetY = targetY;
        this.width = width;
        this.height = height;
        this.durationInSeconds = duration;
    }

    public static void printMovement(Movement movements) {
        System.out.println("Initial Position: (" + movements.initialX + ", " + movements.initialY + ")");
        System.out.println("Target Position: (" + movements.targetX + ", " + movements.targetY + ")");
        System.out.println("Duration: " + movements.durationInSeconds + " seconds");
        System.out.println("Width: " + movements.width);
        System.out.println("Height: " + movements.height);
    }

    public void setInitialX(int initialX) {
        this.initialX = initialX;
    }

    public void setInitialY(int initialY) {
        this.initialY = initialY;
    }

    public int getInitialX() {
        return initialX;
    }

    public int getInitialY() {
        return initialY;
    }

    public int getTargetX() {
        return targetX;
    }

    public int getTargetY() {
        return targetY;
    }

    public int getDurationInSeconds() {
        return durationInSeconds;
    }

    public int getWidth() {
        return width;
    }
    public int getHeight() {
        return height;
    }
}