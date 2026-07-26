/*
  🚨 ResQBot - Robotic Arm Controller 🚨
  Controls a 4-Axis Servo Robotic Arm for debris removal and supply drops.
  Designed for ESP32 using the ESP32Servo library.
*/

#include <ESP32Servo.h>

// ==========================================
// 1. PIN DEFINITIONS (Use Safe ESP32 Pins)
// ==========================================
const int BASE_PIN = 18;     // Rotates the entire arm left/right
const int SHOULDER_PIN = 19; // Moves the lower arm up/down
const int ELBOW_PIN = 23;    // Moves the upper arm up/down
const int GRIPPER_PIN = 32;  // Opens and closes the claw

// ==========================================
// 2. SERVO OBJECTS
// ==========================================
Servo baseServo;
Servo shoulderServo;
Servo elbowServo;
Servo gripperServo;

// ==========================================
// 3. SETUP FUNCTION
// ==========================================
void setup() {
  Serial.begin(115200);
  Serial.println("Initializing ResQBot Arm Systems...");

  // ESP32 requires setting specific PWM timers for servos
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // Attach servos to pins with standard pulse widths (500us to 2400us)
  baseServo.setPeriodHertz(50);
  baseServo.attach(BASE_PIN, 500, 2400);
  
  shoulderServo.setPeriodHertz(50);
  shoulderServo.attach(SHOULDER_PIN, 500, 2400);
  
  elbowServo.setPeriodHertz(50);
  elbowServo.attach(ELBOW_PIN, 500, 2400);
  
  gripperServo.setPeriodHertz(50);
  gripperServo.attach(GRIPPER_PIN, 500, 2400);

  // Move arm to starting safe position
  resetArm();
  delay(2000);
}

// ==========================================
// 4. MAIN LOOP (Test Sequence)
// ==========================================
void loop() {
  Serial.println("Executing Debris Removal Sequence...");
  
  // 1. Move to target
  moveBase(90);      // Face forward
  delay(500);
  
  // 2. Reach down
  reachDown();
  delay(1000);
  
  // 3. Grab object
  closeGripper();
  delay(1000);
  
  // 4. Lift object
  reachUp();
  delay(1000);
  
  // 5. Move to the side and drop
  moveBase(180);     // Turn 90 degrees left
  delay(1000);
  openGripper();     // Drop the debris
  delay(1000);
  
  // 6. Return to resting state
  resetArm();
  
  Serial.println("Sequence Complete. Waiting 5 seconds...");
  delay(5000);
}

// ==========================================
// 5. ARM CONTROL FUNCTIONS
// ==========================================

// Move the base smoothly to a specific angle
void moveBase(int targetAngle) {
  baseServo.write(targetAngle);
}

// Pre-programmed: Reach down to grab
void reachDown() {
  shoulderServo.write(45); // Lower shoulder
  elbowServo.write(120);   // Extend elbow down
}

// Pre-programmed: Lift arm up safely
void reachUp() {
  shoulderServo.write(135); // Raise shoulder
  elbowServo.write(90);     // Level the elbow
}

// Open the claw
void openGripper() {
  gripperServo.write(0); // Adjust this angle based on your specific claw
}

// Close the claw tightly
void closeGripper() {
  gripperServo.write(90); // Adjust this angle so it doesn't crush the object
}

// Fold the arm into a compact, safe driving position
void resetArm() {
  Serial.println("Arm moving to safe driving posture...");
  baseServo.write(90);      // Centered
  shoulderServo.write(90);  // Upright
  elbowServo.write(90);     // Folded in
  openGripper();            // Claw open
}
