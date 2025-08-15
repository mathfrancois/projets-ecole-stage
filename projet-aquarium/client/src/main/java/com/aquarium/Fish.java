package com.aquarium;
import java.util.Objects;
import java.util.Queue;
import java.util.LinkedList;

public class Fish {
    private String name;
    private double x, y;  // Position en % de l’écran
    private double width, height;
    private String moveFunction;
    private Queue<Movement> movementQueue = new LinkedList<>();
    private boolean HavetToBeRemoved = false;



    public Fish(String name, double x, double y, double width, double height, String moveFunction) {
        this.name = name;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.moveFunction = moveFunction;
    }

    public String getName() { return name; }
    public double getX() { return x; }
    public double getY() { return y; }
    public double getWidth() { return width; }
    public double getHeight() { return height; }
    public String getMoveFunction(){ return moveFunction; }
    public boolean getHasToBeRemoved() { return HavetToBeRemoved; }

    public void setHasToBeRemoved(boolean value) { this.HavetToBeRemoved = value; }

    public void setPosition(double x, double y) {
        this.x = x;
        this.y = y;
    }

    public void setX(double x) {
        this.x = x;
    }
    public void setY(double y) {
        this.y = y;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Fish fish = (Fish) obj;
        return Objects.equals(name, fish.name);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name);
    }

    public void addMovement(Movement m) {
        movementQueue.add(m);
    }

    public Movement pollMovement() {
        return movementQueue.poll();
    }

    public boolean hasNextMovement() {
        return !movementQueue.isEmpty();
    }


}