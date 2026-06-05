Perfect! Looking at your code, I can see exactly how everything is connected. You're right - the code itself contains all the wiring information. Let me give you a simplified **single-file README** that explains everything based on your actual code, with only one code file needed.

---

## Ready-to-Paste README.md (Single File Version)

```markdown
# Autonomous Wall-Following to Line-Following Robot

[![Demo Video](https://img.youtube.com/vi/XXXXXXXXXXX/0.jpg)](https://www.youtube.com/watch?v=XXXXXXXXXXX)
> *Click above to watch demo video | Replace `XXXXXXXXXXX` with your YouTube video ID*

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
```

### Sensor Logic Explained

**IR Sensors (Line Detection):**
- `LOW` = Black line detected
- `HIGH` = White surface (or brown floor)
- The robot ignores line detection if ALL sensors see white (brown floor case)

**Ultrasonic Sensors:**
- Front sensor (GPIO 32/33): Detects if wall continues ahead
- Back/Side sensor (GPIO 25/26): Measures distance to side wall
- EMA filter smooths out noisy readings

**State Machine:**
```
START → WALL FOLLOW (5 sec minimum) → Check IR sensors → LINE FOLLOW
                ↑                                              │
                └──────────────────────────────────────────────┘
                          (if line is lost)
```

---

## How to Use This Code

### 1. Install Arduino IDE & ESP32 Board
- Download Arduino IDE from arduino.cc
- Add ESP32 board: File → Preferences → Additional Boards Manager URLs
- Add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Install ESP32 board via Boards Manager

### 2. Required Libraries
None! This code uses only built-in Arduino/ESP32 functions.

### 3. Upload the Code
```bash
1. Open robot.ino in Arduino IDE
2. Select Board: Tools → Board → ESP32 Dev Module
3. Select Port: Tools → Port → (your COM port)
4. Click Upload (→ arrow icon)
```

### 4. Calibration (Optional but Recommended)
Open Serial Monitor (Tools → Serial Monitor) at 115200 baud.
Watch the sensor readings and adjust:
- `TARGET_DIST` if you want different wall distance
- `lineKp` / `lineKd` if line following oscillates
- `Kp/Ki/Kd` if wall following is too wobbly

### 5. Run the Robot
1. Power ESP32 via USB or 5V
2. Power L298N via battery (7.4V-12V)
3. Place robot against a wall
4. The robot will:
   - Follow wall for 5+ seconds
   - When wall ends, wait and check for line
   - Automatically switch to line following
   - Follow black line indefinitely

---

## Demo Video

**[VIDEO_LINK_HERE]**

*Replace with your YouTube link or upload demo_video.mp4 to this repository*

---

## Troubleshooting

| Problem | Likely Fix |
|---------|-------------|
| Motors not moving | Check battery connection to L298N. ESP32 USB alone cannot power motors |
| ESP32 keeps restarting | Use separate power for motors! Common issue |
| Robot ignores line | Check IR sensor wiring. Adjust `WALL_FOLLOW_DELAY` if needed |
| Wall following oscillates | Reduce `Kp` (try 1.5) or increase `Kd` (try 3.0) |
| Line following zig-zags | Reduce `lineKp` (try 30) or increase `lineKd` (try 2.5) |
| Ultrasonic readings unstable | The code already has EMA filter. Check sensor placement (avoid motor interference) |

---

## Repository Contents

| File | Description |
|------|-------------|
| `robot.ino` | Complete Arduino sketch (wall-follow + line-follow + state machine) |
| `README.md` | This file - all documentation |
| `demo_video.mp4` | Demonstration video (add yours here) |

**Note:** No separate calibration file needed! All calibration constants are at the top of `robot.ino` in the "Speeds" and "PID settings" sections.

---

## Future Improvements

- [ ] Add Bluetooth control for manual override
- [ ] Implement adaptive speed (slower on turns)
- [ ] Log sensor data to SD card for analysis
- [ ] Add OLED display to show current mode and sensor values

---

## Author

**[YOUR NAME]**  
Introduction to Robotics – **[YOUR UNIVERSITY]**  
**[SEMESTER / YEAR]**

| Platform | Link |
|----------|------|
| GitHub | [github.com/YOUR_USERNAME](https://github.com/YOUR_USERNAME) |
| LinkedIn | [linkedin.com/in/YOUR_NAME](https://linkedin.com/in/YOUR_NAME) |

---

