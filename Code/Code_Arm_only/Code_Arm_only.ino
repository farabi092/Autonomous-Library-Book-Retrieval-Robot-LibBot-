#include <Servo.h>

// ==================== ARM & GRIPPER SERVO PINS ====================
#define PIN_SERVO_BASE    7
#define PIN_SERVO_MID     8
#define PIN_SERVO_ARM3    9
#define PIN_SERVO_GRIPPER 10

Servo servoBase, servoMid, servoArm3, servoGripper;

// ==================== SERVO CALIBRATION ANGLES ====================
// *** UPDATE THESE WITH YOUR MEASURED ANGLES ***

// Rest Position (Arm folded, safe)
#define BASE_REST_POS    90
#define MID_REST_POS     90
#define ARM3_REST_POS    90

// Grab Position (reaching to side for book)
#define BASE_GRAB_POS    90
#define MID_GRAB_POS     130
#define ARM3_GRAB_POS    70

// Gripper Positions (with Y-axis inversion: 180 - angle)
#define GRIPPER_OPEN     0
#define GRIPPER_CLOSE    180

// Position tracking for smooth servo movements
int currentBase    = BASE_REST_POS;
int currentMid     = MID_REST_POS;
int currentArm3    = ARM3_REST_POS;
int currentGripper = GRIPPER_CLOSE;

bool isHoldingBook = false;

// ==================== SETUP ====================
void setup() {
  Serial.begin(9600);
  Serial.println("=== ARM & GRIPPER INITIALIZING ===");
  
  // Initialize Servos
  Serial.println("Attaching servos...");
  servoBase.attach(PIN_SERVO_BASE);
  servoMid.attach(PIN_SERVO_MID);
  servoArm3.attach(PIN_SERVO_ARM3);
  servoGripper.attach(PIN_SERVO_GRIPPER);
  
  delay(500);
  
  // Move arm to REST position
  Serial.println("Moving to REST position...");
  moveServoSmooth(servoBase, currentBase, BASE_REST_POS, 20);
  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 20);
  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 20);
  moveGripperSmooth(GRIPPER_CLOSE, 20);
  
  delay(500);
  Serial.println("Ready!");
}

// ==================== MAIN LOOP ====================
void loop() {
  // Test sequence
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();
    
    if (cmd == "GRAB") {
      Serial.println("Executing GRAB sequence...");
      executeGrabSequence();
    }
    else if (cmd == "REST") {
      Serial.println("Returning to REST position...");
      returnArmToRest();
    }
    else if (cmd == "OPEN") {
      Serial.println("Opening gripper...");
      moveGripperSmooth(GRIPPER_OPEN, 15);
    }
    else if (cmd == "CLOSE") {
      Serial.println("Closing gripper...");
      moveGripperSmooth(GRIPPER_CLOSE, 15);
    }
    else {
      Serial.println("Commands: GRAB, REST, OPEN, CLOSE");
    }
  }
}

// ==================== ARM & GRIPPER FUNCTIONS ====================

// Smooth servo movement with angle tracking
void moveServoSmooth(Servo &servo, int &currentPos, int targetPos, int stepDelayMs) {
  Serial.print("Moving servo from ");
  Serial.print(currentPos);
  Serial.print(" to ");
  Serial.println(targetPos);
  
  if (currentPos < targetPos) {
    for (int pos = currentPos; pos <= targetPos; pos++) {
      servo.write(pos);
      delay(stepDelayMs);
    }
  } else {
    for (int pos = currentPos; pos >= targetPos; pos--) {
      servo.write(pos);
      delay(stepDelayMs);
    }
  }
  currentPos = targetPos;
}

// Gripper control with Y-axis inversion (180 - angle)
void moveGripperSmooth(int targetPos, int stepDelayMs) {
  int invertedTarget = 180 - targetPos;
  Serial.print("Moving gripper from ");
  Serial.print(currentGripper);
  Serial.print(" to ");
  Serial.println(invertedTarget);
  
  if (currentGripper < invertedTarget) {
    for (int pos = currentGripper; pos <= invertedTarget; pos++) {
      servoGripper.write(pos);
      delay(stepDelayMs);
    }
  } else {
    for (int pos = currentGripper; pos >= invertedTarget; pos--) {
      servoGripper.write(pos);
      delay(stepDelayMs);
    }
  }
  currentGripper = invertedTarget;
}

// Execute grab sequence
void executeGrabSequence() {
  Serial.println("Step 1: Opening gripper...");
  moveGripperSmooth(GRIPPER_OPEN, 15);
  delay(300);

  Serial.println("Step 2: Lowering mid joint...");
  moveServoSmooth(servoMid, currentMid, MID_GRAB_POS, 20);
  delay(200);

  Serial.println("Step 3: Lowering upper joint...");
  moveServoSmooth(servoArm3, currentArm3, ARM3_GRAB_POS, 20);
  delay(400);

  Serial.println("Step 4: Closing gripper on book...");
  moveGripperSmooth(GRIPPER_CLOSE, 20);
  delay(500);

  Serial.println("Step 5: Lifting upper joint...");
  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 20);
  delay(200);

  Serial.println("Step 6: Lifting mid joint...");
  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 20);
  delay(300);

  isHoldingBook = true;
  Serial.println("Book grabbed successfully!");
}

// Return arm to rest position
void returnArmToRest() {
  Serial.println("Step 1: Opening gripper...");
  moveGripperSmooth(GRIPPER_OPEN, 15);
  delay(300);

  Serial.println("Step 2: Returning arm joints to rest...");
  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 15);
  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 15);
  moveServoSmooth(servoBase, currentBase, BASE_REST_POS, 15);
  delay(300);

  Serial.println("Step 3: Closing gripper...");
  moveGripperSmooth(GRIPPER_CLOSE, 15);

  isHoldingBook = false;
  Serial.println("Arm returned to REST position!");
} 
