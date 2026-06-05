# Wall-following-and-line-folowing
Arduino code for a robot that autonomously navigates using 5 IR sensors for line following and an ultrasonic sensor for wall following.

---

## Project Overview

This project was developed as the final assignment for my **Introduction to Robotics** course. The goal was to design a single autonomous robot capable of navigating two distinct environments without human intervention.

**The Core Challenge:** The robot starts in a corridor where it must follow a wall. When the corridor ends (the wall disappears), the robot must automatically detect this change, switch modes, and begin following a line on the floor.

**Key Achievement:** Successfully implemented a **finite state machine** on an ESP32 that transitions between `WALL_FOLLOW` and `LINE_FOLLOW` states based on real-time sensor data.

---

## Demo Video

**[https://www.linkedin.com/posts/muhammad-umar-470205377_robotics-esp32-embeddedsystems-ugcPost-7426727320171741184-qKT_?utm_source=share&utm_medium=member_desktop&rcm=ACoAAF0c7FQBAq2rTRdLSm2bP4Ak0SG9Vtf9ipY]**  



---

## How It Works: Detailed Logic

### State 1: Wall Following Mode

| Component | Purpose |
|-----------|---------|
| Two HC-SR04 Ultrasonic Sensors | Measure distance to wall from front and side |

**Logic:** The robot maintains a constant distance from the wall. If it gets too close, it steers away. If it gets too far, it steers towards the wall.

**Why two sensors?** Using two sonar sensors provides redundancy and allows the robot to "see" if the wall is angling towards or away from it, enabling smoother corrections compared to a single sensor.

### The Transition Trigger (Wall Ends)

The robot continuously monitors the front sonar sensor.

| Condition | Action |
|-----------|--------|
| Front sonar reading > 100cm AND side sensor detects no wall | Robot deduces wall has ended → switches to `LINE_FOLLOW` mode |

**Safety Feature:** A debounce timer requires 3 consistent readings over 200ms before switching states. This prevents false triggers from sensor noise or temporary gaps.

### State 2: Line Following Mode

| Component | Purpose |
|-----------|---------|
| 5-Channel IR Sensor Array | Detect position of black line on white surface |

**Logic:** The ESP32 reads which sensors are over the line and adjusts motor speeds using **PID (Proportional-Integral-Derivative)** control to keep the robot centered.

**Result:** Smooth, oscillation-free line following even at moderate speeds.

---

## Hardware Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32 Dev Board | 1 | Main microcontroller (computation, dual-core processing) |
| HC-SR04 Ultrasonic Sensors | 2 | Wall detection and distance measurement |
| 5-Channel IR Sensor Array | 1 | Line detection |
| L298N Motor Driver | 1 | Controlling speed and direction of motors |
| BO Motors with Wheels | 2 | Actuation / Movement |
| Battery (7.4V - 12V) | 1 | Power supply |

---

## Repository Contents

| File | Description |
|------|-------------|
| `/code/robot_fsm.ino` | Main Arduino sketch with state machine logic |
| `/code/sensor_calibration.ino` | Script to calibrate IR sensors and sonar thresholds |
| `/wiring_diagram.png` | Schematic showing all connections |
| `/demo_video.mp4` | Demonstration video (add your video file here) |

---

## Wiring Diagram (Quick Reference)

| ESP32 Pin | Connected To |
|-----------|---------------|
| D2 | Trig Pin (Sonar 1 - Front) |
| D3 | Echo Pin (Sonar 1 - Front) |
| D4 | Trig Pin (Sonar 2 - Side) |
| D5 | Echo Pin (Sonar 2 - Side) |
| D12 | IR Sensor 1 (Leftmost) |
| D13 | IR Sensor 2 |
| D14 | IR Sensor 3 (Center) |
| D15 | IR Sensor 4 |
| D16 | IR Sensor 5 (Rightmost) |
| D18 | L298N - IN1 |
| D19 | L298N - IN2 |
| D21 | L298N - IN3 |
| D22 | L298N - IN4 |
| D23 | L298N - ENA (PWM for speed control) |
| D25 | L298N - ENB (PWM for speed control) |
| GND | Common ground (ESP32, sensors, L298N) |
| VIN / 5V | Power for sensors |

---

## How to Run This Project

### Prerequisites
- Arduino IDE with ESP32 board support installed
- Required libraries:
  - `NewPing` (for ultrasonic sensors)
  - `PID` (for line following, optional if you write your own)

### Step-by-Step Instructions

1. **Clone the repository**
   ```bash
   git clone https://github.com/[YOUR_USERNAME]/[YOUR_REPOSITORY_NAME].git
