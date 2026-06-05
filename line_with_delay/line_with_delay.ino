#include <WiFi.h>

// Motor pins
#define IN1 4
#define IN2 5
#define ENA 14
#define IN3 12
#define IN4 13
#define ENB 27

// Sonar pins
#define FR_TRIG 32
#define FR_ECHO 33
#define BR_TRIG 25
#define BR_ECHO 26
// IR sensors pins
#define IR1 15   // Left
#define IR2 16
#define IR3 17   // Center
#define IR4 18
#define IR5 19   // Right

// Speeds
int leftSpeed = 80;
int rightSpeed = 80;     // normal cruising speed (PWM)

#define MIN_SPEED  0    // never allow motor to stop
#define MAX_SPEED  200
#define IR_MIN 0

// Line following settings
int lineBaseSpeed = 100;      // Base speed for line following
int lineKp = 60;              // Line following proportional gain
float lineKd = 1.75;
// Flags
bool autoMode = true;
bool isMoving = false;
int moveDir = 0;
bool lineMode = false;   // Line-follow mode
bool lineDetected = false;
unsigned long lineDetectionStartTime = 0;
bool wallFollowTimerStarted = false; 
unsigned long wallFollowStartTime = 0;
const unsigned long WALL_FOLLOW_DELAY = 5000; // 10 seconds delay

// PID settings
const int SONAR_SAMPLES = 4;
float Kp = 3.2; 
float Ki = 0.02;
float Kd = 2.6;
float integral = 0;
float lastError = 0;
const int TARGET_DIST = 15;
const unsigned long WALL_DT = 80;
unsigned long lastWallTime = 0;

// ---------- Set PWM Frequency ----------
void setPwmFrequency() {
  // Initialize motor pins first (this sets up the PWM channels)
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  
  // Now set frequency FOR EACH PIN individually:
  analogWriteFrequency(ENA, 100);  // Set 100Hz for ENA pin (GPIO14)
  analogWriteFrequency(ENB, 100);  // Set 100Hz for ENB pin (GPIO27)
  
  // Set resolution FOR EACH PIN:
  analogWriteResolution(ENA, 8);   // 8-bit resolution for ENA (0-255)
  analogWriteResolution(ENB, 8);   // 8-bit resolution for ENB (0-255)
  
  Serial.println("PWM set to 100Hz, 8-bit resolution for both motors");
}

// ---------- Median filtered sonar ----------
long readSonarFast(int trigPin, int echoPin) {
  static long filtered[40];  // indexed by pin number (safe on ESP32)

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  unsigned long dur = pulseIn(echoPin, HIGH, 18000); // shorter timeout
  long cm = (dur == 0) ? 400 : (dur * 0.034 / 2);

  // -------- EMA FILTER --------
  // 0.7 old + 0.3 new → fast but stable
  filtered[echoPin] = (filtered[echoPin] == 0)
                        ? cm
                        : (filtered[echoPin] * 4 + cm * 6) / 10;

  return filtered[echoPin];
}

// ---------- Motor Helpers ----------
void stopMotors() {
  isMoving = false;
  moveDir = 0;
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void applySpeeds(int L, int R) {
  if (isMoving || autoMode || lineMode) {
    analogWrite(ENA, constrain(L, MIN_SPEED, MAX_SPEED));
    analogWrite(ENB, constrain(R, MIN_SPEED, MAX_SPEED));
  } else {
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }
}

void forwardCmd() {
  autoMode = false; isMoving = true; moveDir = 1;
  lineMode = false;
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  applySpeeds(leftSpeed, rightSpeed);
}

// ---------- Line Follow ----------
void followLine() {
  int sensors[5] = {
    digitalRead(IR1),
    digitalRead(IR2),
    digitalRead(IR3),
    digitalRead(IR4),
    digitalRead(IR5)
  };

  int weights[5] = {-2, -1, 0, 1, 2};

  int error = 0;
  int active = 0;

  for (int i = 0; i < 5; i++) {
    if (sensors[i] == LOW) {  // LOW = black line detected
      error += weights[i];
      active++;
    }
  }

  // Debug output
  Serial.print("Sensors: ");
  for(int i = 0; i < 5; i++) Serial.print(sensors[i]);
  Serial.print(" | Error: "); Serial.print(error);
  Serial.print(" | Active: "); Serial.println(active);

  if (active == 0) {
    // Line lost - stop or search
    applySpeeds(0, 0);
    Serial.println("Line lost!");
    return;
  }

  error /= active;  // Average error
  
  // -------- ADD PD CONTROL HERE --------
  static int lastError = 0;          // Store previous error for derivative
  static unsigned long lastTime = 0; // Store previous time
  
  unsigned long currentTime = millis();
  float dt = (currentTime - lastTime) / 1000.0; // Time in seconds
  if (dt == 0) dt = 0.01; // Avoid division by zero on first run
  
  // Calculate derivative (rate of change of error)
  float derivative = (error - lastError) / dt;
  
  // PD Control: correction = Kp*error + Kd*derivative
  int correction = lineKp * error + lineKd * derivative;
  
  // Update for next iteration
  lastError = error;
  lastTime = currentTime;
  // -------------------------------------

  int leftMotor = lineBaseSpeed + correction;
  int rightMotor = lineBaseSpeed - correction;

  // Debug - show PD terms
  Serial.print("Error: "); Serial.print(error);
  Serial.print(" | Deriv: "); Serial.print(derivative);
  Serial.print(" | Correction: "); Serial.print(correction);
  Serial.print(" | L: "); Serial.print(leftMotor);
  Serial.print(" | R: "); Serial.println(rightMotor);

  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);

  // Apply speed constraints
  applySpeeds(
    constrain(leftMotor, IR_MIN, MAX_SPEED),
    constrain(rightMotor, IR_MIN, MAX_SPEED)
  );
}

void checkForLine() {
  // Start wall-follow timer when autoMode is first enabled
  if (autoMode && !wallFollowTimerStarted) {
    wallFollowTimerStarted = true;
    wallFollowStartTime = millis();
    Serial.println("Wall-follow timer started (10s delay)");
  }
  
  // Reset timer if autoMode is turned off
  if (!autoMode && wallFollowTimerStarted) {
    wallFollowTimerStarted = false;
  }

  // Check if ANY IR sensor detects line (LOW = black line)
  // First, check if ALL sensors see white (brown floor)
  if (digitalRead(IR1) == HIGH && digitalRead(IR2) == HIGH && 
      digitalRead(IR3) == HIGH && digitalRead(IR4) == HIGH && 
      digitalRead(IR5) == HIGH) {
    // Brown floor detected (all sensors see white) - ignore
    lineDetected = false;
  }
  else if (digitalRead(IR1) == LOW || digitalRead(IR2) == LOW || 
           digitalRead(IR3) == LOW || digitalRead(IR4) == LOW || 
           digitalRead(IR5) == LOW) {
    
    // Check if wall-follow delay period has passed
    unsigned long wallFollowElapsed = millis() - wallFollowStartTime;
    bool delayPeriodPassed = wallFollowTimerStarted && (wallFollowElapsed > WALL_FOLLOW_DELAY);
    
    // Only process line detection AFTER the 10-second delay OR if not in wall-follow mode
    if (!wallFollowTimerStarted || delayPeriodPassed) {
      
      if (!lineDetected) {
        // First detection - start timer
        lineDetected = true;
        lineDetectionStartTime = millis();
        Serial.println("Line detected, starting verification...");
      } 
      else if (millis() - lineDetectionStartTime > 250) {
        // Line detected continuously for 250ms - switch to line mode
        if (!lineMode && autoMode) {
          lineMode = true;
          autoMode = false;
          wallFollowTimerStarted = false; // Reset timer
          Serial.println("Auto-switched to Line Mode!");
        }
      }
    }
    else {
      // Still in wall-follow delay period - ignore line detections
      if (lineDetected) {
        lineDetected = false;
        Serial.println("Line ignored (wall-follow delay active)");
      }
    }
  } 
  else {
    // No line detected (mixed signals or all white)
    lineDetected = false;
  }
}

// ---------- PID Wall-Follow ----------
void followWallPID() {
  static unsigned long lastTime = 0;
  unsigned long now = millis();
  if (now - lastTime < WALL_DT) return;
  lastTime = now;
  
  checkForLine();
  if (lineMode) {
    return; // Let followLine() handle in main loop
  }
  // -------- READ SONARS (FAST) --------
  float fr = readSonarFast(FR_TRIG, FR_ECHO);
  float br = readSonarFast(BR_TRIG, BR_ECHO);

  // -------- ANGLE ERROR ONLY --------
  // positive → front farther → turn LEFT
  float error = TARGET_DIST - (fr + br)/2;

  float dt = WALL_DT / 1000.0;

  // -------- PID (STEERING ONLY) --------
  integral += error * dt;
  integral = constrain(integral, -30, 30);

  float derivative = (error - lastError) / dt;
  lastError = error;

  float turn = Kp * error + Ki * integral + Kd * derivative;
  turn = constrain(turn, -30, 30);   // very soft correction

  // -------- CONSTANT BASE SPEED --------
  int base = min(leftSpeed, rightSpeed);   // balanced speed
  base = constrain(base, MIN_SPEED, MAX_SPEED);

  // -------- MOTOR MIXING --------
  int L = leftSpeed - turn;
  int R = rightSpeed + turn;

  // -------- DRIVE FORWARD --------
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);

  applySpeeds(L, R);
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(FR_TRIG, OUTPUT); pinMode(FR_ECHO, INPUT);
  pinMode(BR_TRIG, OUTPUT); pinMode(BR_ECHO, INPUT);
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  pinMode(IR3, INPUT);
  pinMode(IR4, INPUT);
  pinMode(IR5, INPUT);

  setPwmFrequency();
  stopMotors();
  
  Serial.println("ESP32 Car Controller Ready");
}

// ---------- Main Loop ----------
void loop() {
  if (autoMode) followWallPID();
  if (lineMode) followLine();
}