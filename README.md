# Maze Solver Robot (Arduino)

An autonomous **maze solving robot** built using **Arduino, ultrasonic sensors, and a motor driver**.
The robot detects walls in the **left, front, and right directions** and navigates the maze using a **left-wall follower algorithm**. Based on the detected obstacles, it decides whether to move forward, turn, or stop.

The project demonstrates basic concepts of **robotics, embedded systems, and sensor-based navigation**.

---

## Features

* Autonomous maze navigation
* Obstacle detection using **3 ultrasonic sensors**
* **Left wall following algorithm**
* Pivot turning using DC motors
* Serial monitor debugging

---

## Hardware Used

* Arduino UNO / Nano
* 3 × HC-SR04 Ultrasonic Sensors
* L298N / TB6612FNG Motor Driver
* 2 × DC Motors with robot chassis
* Battery pack
* Jumper wires

---

## Pin Configuration

### Ultrasonic Sensors

| Sensor | TRIG | ECHO |
| ------ | ---- | ---- |
| Left   | 7    | 6    |
| Front  | 9    | 8    |
| Right  | 5    | 4    |

### Motor Driver

| Pin | Arduino |
| --- | ------- |
| IN1 | 2       |
| IN2 | 3       |
| IN3 | 12      |
| IN4 | 13      |
| ENA | 10      |
| ENB | 11      |

---

## Navigation Logic

| Condition             | Action       |
| --------------------- | ------------ |
| No walls              | Move Forward |
| All walls blocked     | Stop         |
| Front + Left blocked  | Turn Right   |
| Front + Right blocked | Turn Left    |
| Front blocked         | Turn Left    |
| Right wall only       | Move Left    |
| Otherwise             | Move Forward |


