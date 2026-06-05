# Autonomous Wall-Following to Line-Following Robot

![Demo Video](https://drive.google.com/file/d/13YyQm-kN6D45qmC-4oXycSxImgixIGdW/view?usp=sharing)

---

## Project Overview

This robot was developed as my final project for **Introduction to Robotics**. It autonomously navigates a course by first following a wall, then automatically switching to line-following mode when the wall ends.

**Key Achievement:** Seamless state transition between two navigation modes using a finite state machine on ESP32.

### How It Works (Simple Version)

| Mode | What Happens | Sensors Used |
|------|--------------|--------------|
| **Wall Following** | Robot maintains 15cm distance from wall | 2x Ultrasonic (front & back/side) |
| **Transition** | When wall ends, robot waits 10 seconds then checks for line | IR Array |
| **Line Following** | Robot follows black line on white surface | 5x IR Sensors |

---

## Hardware Connections

Based on the actual code, here's exactly how everything connects:

### ESP32 Pin Mapping

| Component | Pin | Purpose |
|-----------|-----|---------|
| **Motor Driver L298N** | | |
| IN1 | GPIO 4 | Motor A direction |
| IN2 | GPIO 5 | Motor A direction |
| ENA | GPIO 14 | Motor A speed (PWM) |
| IN3 | GPIO 12 | Motor B direction |
| IN4 | GPIO 13 | Motor B direction |
| ENB | GPIO 27 | Motor B speed (PWM) |
| **Ultrasonic Sensors (HC-SR04)** | | |
| Front TRIG | GPIO 32 | Send pulse |
| Front ECHO | GPIO 33 | Receive pulse |
| Back/Side TRIG | GPIO 25 | Send pulse |
| Back/Side ECHO | GPIO 26 | Receive pulse |
| **IR Sensor Array (5-Channel)** | | |
| IR1 (Leftmost) | GPIO 15 | Line detection |
| IR2 | GPIO 16 | Line detection |
| IR3 (Center) | GPIO 17 | Line detection |
| IR4 | GPIO 18 | Line detection |
| IR5 (Rightmost) | GPIO 19 | Line detection |

### Power Configuration
- **ESP32 & Sensors:** Powered via USB or 5V pin
- **Motors & L298N:** Separate 7.4V-12V battery (prevents ESP32 resets)
- **PWM Frequency:** 100Hz with 8-bit resolution (0-255 speed values)

---

## Code Explanation

The entire robot logic is contained in **one Arduino sketch** (`robot.ino`). Here's what the main functions do:

| Function | What It Does |
|----------|---------------|
| `setup()` | Initializes pins, sets PWM frequency, starts serial monitor |
| `followWallPID()` | Main wall-following logic with PID control. Target distance = 15cm |
| `followLine()` | Line-following with PD control. Uses 5 IR sensors |
| `checkForLine()` | Monitors IR sensors. Triggers mode switch after 10 seconds |
| `readSonarFast()` | Reads ultrasonic sensor with EMA filter for smooth data |
| `applySpeeds()` | Sets motor speeds with constraints (0-200 range) |

### Key Parameters You Can Adjust

```cpp
// Wall Following (PID)
#define TARGET_DIST 15        // Desired distance from wall (cm)
Kp = 3.2;                     // Proportional gain
Ki = 0.02;                    // Integral gain  
Kd = 2.6;                     // Derivative gain

// Line Following (PD)
lineBaseSpeed = 100;          // Base speed (0-200)
lineKp = 60;                  // Proportional gain
lineKd = 1.75;                // Derivative gain

// Timing
WALL_FOLLOW_DELAY = 5000;     // Wait 5 seconds before allowing line detection
lineDetectionStartTime = 250; // Verify line for 250ms before switching
