// === Ultrasonic Sensor Pins ===
#define FL_TRIG 7
#define FL_ECHO 6
#define F_TRIG 9
#define F_ECHO 8
#define FR_TRIG 5
#define FR_ECHO 4

// === Motor Driver (L298N / TB6612FNG) Pins ===
#define IN1 2
#define IN2 3
#define IN3 12
#define IN4 13
#define ENA 10    // Left Motor PWM
#define ENB 11    // Right Motor PWM

// === Maze & PID Constants ===
// *** NEW SPEED CONSTANTS ***

// *** FIX ***
// Set to a REALISTIC value. 5 is too low and will stall the motors.
// Start with 40. If motors just hum, INCREASE this to 45 or 50.
// If it's too fast, you can try 35, but any lower will likely stop.
const int M_LEFT_SPEED = 80;  
const int M_RIGHT_SPEED = 80; 

// These control the "on/off" pulse for moving forward
const int PULSE_ON_MS = 60;     // Time motors are ON
const int PULSE_OFF_MS = 140;   // Time motors are OFF (make this larger for slower speed)

const float wallSetpoint = 10.0;    // Desired left wall distance (cm)
// *** FIX *** Set front threshold equal to side threshold for reliability
const float frontThreshold = 10.0;    // Obstacle detection threshold (cm)
const float sideThreshold = 10.0;     // Threshold for detecting side walls

// === PID Constants ===
float Kp = 2;
float Ki = 0;
float Kd = 0;

// === PID Variables ===
float lastError = 0;
float integral = 0;

// === Distance Utility ===
float readDistance(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  
  long duration = pulseIn(echo, HIGH, 10000);
  float distance = duration * 0.0345 / 2;
  
  if (distance <= 0.0) distance = 400;  // no echo = far away
  return min(distance, 30.0f);
}

// === SIMPLIFIED Motor Control - Always Forward ===
void setMotor(int leftSpeed, int rightSpeed) {
  // ALWAYS SET FORWARD DIRECTION
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  
  analogWrite(ENA, leftSpeed);
  analogWrite(ENB, rightSpeed);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// === Turning Functions - *** FIXED PIVOT TURN LOGIC *** ===
void turnLeft() {
  Serial.println("Turning LEFT (Pivot)");
  
  // Left motor backward
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  // Right motor FORWARD
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 8); // Use consistent slow speed
  
  // You may need to adjust this delay to get a 90-degree turn
  delay(150); 
  
  stopMotors();
  delay(100);
}

void turnRight() {
  Serial.println("Turning RIGHT (Pivot)");
  
  // Left motor FORWARD
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA,  8); // Use consistent slow speed

  // Right motor BACKWARD
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
  
  // You may need to adjust this delay to get a 90-degree turn
  delay(100); 
  
  stopMotors();
  delay(50);
}

// === Move Forward Helper - *** NEW PULSING LOGIC *** ===
void moveForwardDirect() {
  // Turn motors ON
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, M_LEFT_SPEED); 
  analogWrite(ENB, M_RIGHT_SPEED);
  
  // Wait while motors are on
  delay(PULSE_ON_MS);
  
  // Turn motors OFF
  stopMotors();
  
  // Wait while motors are off
  delay(PULSE_OFF_MS);
}

// === Check if wall is present ===
bool isWallPresent(float distance) {
  return distance < sideThreshold;
}

bool isFrontWallPresent(float distance) {
  return distance < frontThreshold;
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  Serial.println("=== MAZE SOLVER ROBOT - PULSING ===");

  // Initialize ultrasonic sensors
  pinMode(FL_TRIG, OUTPUT);
  pinMode(FL_ECHO, INPUT);
  pinMode(F_TRIG, OUTPUT);
  pinMode(F_ECHO, INPUT);
  pinMode(FR_TRIG, OUTPUT);
  pinMode(FR_ECHO, INPUT);

  // Initialize motor pins
  pinMode(IN1, OUTPUT);  
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);  
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);  
  pinMode(ENB, OUTPUT);

  // Ensure motors are stopped initially
  stopMotors();
  
  delay(1000);
}

// === Main Loop ===

void loop() {
  // --- Read Sensors ---
  float leftDist = readDistance(FL_TRIG, FL_ECHO);
  delay(5);
  float frontDist = readDistance(F_TRIG, F_ECHO);
  delay(5);
  float rightDist = readDistance(FR_TRIG, FR_ECHO);
  delay(5);

  // Determine wall presence
  bool leftWall = isWallPresent(leftDist);
  bool frontWall = isFrontWallPresent(frontDist);
  bool rightWall = isWallPresent(rightDist);

  // Debug output
  Serial.print("L:"); Serial.print(leftDist); Serial.print(leftWall ? "(W)" : "(O)");
  Serial.print(" F:"); Serial.print(frontDist); Serial.print(frontWall ? "(W)" : "(O)");
  Serial.print(" R:"); Serial.print(rightDist); Serial.print(rightWall ? "(W)" : "(O)");
  Serial.print(" -> ");



  // === LEFT WALL FOLLOWER LOGIC ===

  // 1. ALL walls blocked → STOP
  if (frontWall && leftWall && rightWall) {
    Serial.println("RULE: ALL BLOCKED - STOP");
    stopMotors();
    delay(500);
    return;
  }

  // 2. Front + Left blocked & Right open → TURN RIGHT
  else if (frontWall && leftWall && !rightWall) {
    Serial.println("RULE: FRONT+LEFT BLOCKED - TURN RIGHT");
    stopMotors();
    delay(200);
    turnRight();
    return;
  }

  // 3. Front + Right blocked → TURN LEFT
  else if (frontWall && rightWall) {
    Serial.println("RULE: FRONT+RIGHT BLOCKED - TURN LEFT");
    stopMotors();
    delay(200);
    turnLeft();
    return;
  }

  // 4. Front blocked only → TURN LEFT (left follower preference)
  else if (frontWall && !leftWall && !rightWall) {
    Serial.println("RULE: FRONT BLOCKED - TURN LEFT");
    stopMotors();
    delay(200);
    turnLeft();
    return;
  }

  // 5. Right blocked & Front + Left open → MOVE LEFT
  else if (rightWall && !frontWall && !leftWall) {
    Serial.println("RULE: RIGHT BLOCKED - MOVE LEFT");
    stopMotors();
    delay(100);
    turnLeft();
    return;
  }

  // 6. ALL forward-allowed cases → MOVE FORWARD
  // Covers:
  // - No walls
  // - Left+Right blocked & front open
  // - Left blocked only
  // - Right blocked only
  else {
    Serial.println("RULE: MOVE FORWARD");
    moveForwardDirect();
  }

}
