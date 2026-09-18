#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


// ==================== CONFIGURATION & TOGGLES ====================
const int BLACK_LINE_STATE = LOW;   // Active-LOW logic (White = 1, Black = 0)
const bool SWAP_IR_SENSORS = false; // Set true if physical left/right IR sensors are swapped


// ==================== PIN DEFINITIONS ====================
#define IN1 5   // Right Motor PWM / Speed
#define IN2 2   // Right Motor Direction
#define IN3 6   // Left Motor PWM / Speed
#define IN4 4   // Left Motor Direction


#define IR_LEFT   A0
#define IR_CENTER A1
#define IR_RIGHT  A2


#define TRIG_PIN 3
#define ECHO_PIN A5
#define BUZZER_PIN 1 // TX Pin (Obstacle alarm only)


#define SS_PIN  A3  // RFID SDA
#define RST_PIN A4  // RFID RST


#define PIN_SERVO_BASE    7
#define PIN_SERVO_MID     8
#define PIN_SERVO_ARM3    9
#define PIN_SERVO_GRIPPER 10


// ==================== SERVO POSITIONS & OBJECTS ====================
Servo servoBase, servoMid, servoArm3, servoGripper;


#define BASE_REST_POS    90
#define MID_REST_POS     90
#define ARM3_REST_POS    90


#define BASE_GRAB_POS    90
#define MID_GRAB_POS     130
#define ARM3_GRAB_POS    70


#define GRIPPER_OPEN     0
#define GRIPPER_CLOSE    180


int currentBase    = BASE_REST_POS;
int currentMid     = MID_REST_POS;
int currentArm3    = ARM3_REST_POS;
int currentGripper = GRIPPER_CLOSE;


bool isHoldingBook = false;


// ==================== SLOW-SPEED PID PARAMETERS ====================
float Kp = 18.0;  
float Kd = 8.0;  
float Ki = 0.0;  


int BASE_SPEED = 65;  // Reduced cruise speed
int MIN_SPEED  = 40;  // Minimum speed threshold
int MAX_SPEED  = 95; // Maximum speed ceiling


float error = 0, lastError = 0, integral = 0;


// ==================== RFID & STATE MACHINE ====================
MFRC522 rfid(SS_PIN, RST_PIN);


byte LIBRARIAN_UID[4] = {0xA3, 0x3D, 0x3D, 0xFC};
byte SHELF_UID[4]     = {0x33, 0x45, 0x3F, 0xFC};
byte BOOK_UID[4]      = {0x49, 0xDB, 0x07, 0x05};


enum State {
  WAIT_FOR_START,
  FOLLOW_TO_SHELF,
  FOLLOW_TO_BOOK,
  FOLLOW_WITH_BOOK
};


State currentState = WAIT_FOR_START;
unsigned long lastRFIDCheck = 0;


// Function Prototypes
void moveServoSmooth(Servo &servo, int &currentPos, int targetPos, int stepDelayMs);
void moveGripperSmooth(int targetPos, int stepDelayMs);
void executeGrabSequence();
void returnArmToRest();
void stopMotors();
void followLinePID();
void setMotorsForward(int leftSpeed, int rightSpeed);
void turnLeft90();
bool checkObstacle();
void handleObstacle();
bool checkRFIDThrottled(byte *targetUID);
bool checkRFID(byte *targetUID);


// ==================== SETUP ====================
void setup() {
  // NOTE: Serial.begin() is omitted intentionally because Pin 1 (TX) is connected
  // to the Buzzer. Serial communication toggles TX pin and creates buzzer noise.


  // Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();


  // Sensors & Buzzer
  pinMode(IR_LEFT, INPUT); pinMode(IR_CENTER, INPUT); pinMode(IR_RIGHT, INPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Keep buzzer silent on power-up


  // RFID Setup
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);


  // Servo Setup
  servoBase.attach(PIN_SERVO_BASE);
  servoMid.attach(PIN_SERVO_MID);
  servoArm3.attach(PIN_SERVO_ARM3);
  servoGripper.attach(PIN_SERVO_GRIPPER);


  delay(300);


  // Initialize Arm to Rest Position
  moveServoSmooth(servoBase, currentBase, BASE_REST_POS, 15);
  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 15);
  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 15);
  moveGripperSmooth(GRIPPER_CLOSE, 15);
}


// ==================== MAIN LOOP ====================
void loop() {
  // Global Librarian Tag Check (Toggle Start / Stop)
  if (checkRFIDThrottled(LIBRARIAN_UID)) {
    if (currentState == WAIT_FOR_START) {
      currentState = FOLLOW_TO_SHELF;
    } else {
      stopMotors();
      currentState = WAIT_FOR_START;
    }
    return;
  }


  // State Machine
  switch (currentState) {
    case WAIT_FOR_START:
      stopMotors();
      break;


    case FOLLOW_TO_SHELF:
      if (checkObstacle()) {
        handleObstacle();
      } else if (checkRFIDThrottled(SHELF_UID)) {
        stopMotors();
        turnLeft90();
        currentState = FOLLOW_TO_BOOK;
      } else {
        followLinePID();
      }
      break;


    case FOLLOW_TO_BOOK:
      if (checkObstacle()) {
        handleObstacle();
      } else if (checkRFIDThrottled(BOOK_UID)) {
        stopMotors();
        executeGrabSequence(); // Automatically pick up the book
        currentState = FOLLOW_WITH_BOOK;
      } else {
        followLinePID();
      }
      break;


    case FOLLOW_WITH_BOOK:
      if (checkObstacle()) {
        handleObstacle();
      } else {
        followLinePID();
      }
      break;
  }
}


// ==================== ARM & GRIPPER FUNCTIONS ====================


void moveServoSmooth(Servo &servo, int &currentPos, int targetPos, int stepDelayMs) {
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


void moveGripperSmooth(int targetPos, int stepDelayMs) {
  int invertedTarget = 180 - targetPos;
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


void executeGrabSequence() {
  moveGripperSmooth(GRIPPER_OPEN, 15);
  delay(300);


  moveServoSmooth(servoMid, currentMid, MID_GRAB_POS, 20);
  delay(200);


  moveServoSmooth(servoArm3, currentArm3, ARM3_GRAB_POS, 20);
  delay(400);


  moveGripperSmooth(GRIPPER_CLOSE, 20);
  delay(500);


  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 20);
  delay(200);


  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 20);
  delay(300);


  isHoldingBook = true;
}


void returnArmToRest() {
  moveGripperSmooth(GRIPPER_OPEN, 15);
  delay(300);


  moveServoSmooth(servoArm3, currentArm3, ARM3_REST_POS, 15);
  moveServoSmooth(servoMid, currentMid, MID_REST_POS, 15);
  moveServoSmooth(servoBase, currentBase, BASE_REST_POS, 15);
  delay(300);


  moveGripperSmooth(GRIPPER_CLOSE, 15);
  isHoldingBook = false;
}


// ==================== PID LINE FOLLOWING ====================


void followLinePID() {
  int rawL = (digitalRead(IR_LEFT) == BLACK_LINE_STATE) ? 1 : 0;
  int C    = (digitalRead(IR_CENTER) == BLACK_LINE_STATE) ? 1 : 0;
  int rawR = (digitalRead(IR_RIGHT) == BLACK_LINE_STATE) ? 1 : 0;


  int L = SWAP_IR_SENSORS ? rawR : rawL;
  int R = SWAP_IR_SENSORS ? rawL : rawR;


  if (L == 1 && C == 0 && R == 0)      error = -2.0;
  else if (L == 1 && C == 1 && R == 0) error = -1.0;
  else if (L == 0 && C == 1 && R == 0) error = 0.0;  
  else if (L == 0 && C == 1 && R == 1) error = 1.0;  
  else if (L == 0 && C == 0 && R == 1) error = 2.0;  
  else if (L == 0 && C == 0 && R == 0) {
    error = (lastError > 0) ? 1.5 : -1.5;
  }


  integral += error;
  integral = constrain(integral, -5, 5);
  float derivative = error - lastError;
  float correction = (Kp * error) + (Ki * integral) + (Kd * derivative);
  lastError = error;


  int leftSpeed  = BASE_SPEED + correction;
  int rightSpeed = BASE_SPEED - correction;


  setMotorsForward(leftSpeed, rightSpeed);
}


// ==================== MOTOR CONTROLS ====================


void setMotorsForward(int leftSpeed, int rightSpeed) {
  leftSpeed  = constrain(leftSpeed, MIN_SPEED, MAX_SPEED);
  rightSpeed = constrain(rightSpeed, MIN_SPEED, MAX_SPEED);


  analogWrite(IN1, rightSpeed);
  digitalWrite(IN2, LOW);


  analogWrite(IN3, leftSpeed);
  digitalWrite(IN4, LOW);
}


void turnLeft90() {
  stopMotors();
  delay(200);
 
  analogWrite(IN1, 85);  // Lowered from 110 (Right motor)
  digitalWrite(IN2, LOW);
  analogWrite(IN3, 60);  // Lowered from 80 (Left motor reverse)
  digitalWrite(IN4, HIGH);
 
  delay(750);            // You may need to slightly increase delay if speed is lower
  stopMotors();
  delay(200);
}


void stopMotors() {
  analogWrite(IN1, 0); digitalWrite(IN2, LOW);
  analogWrite(IN3, 0); digitalWrite(IN4, LOW);
}


// ==================== SENSOR & RFID UTILITIES ====================


bool checkObstacle() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);


  long duration = pulseIn(ECHO_PIN, HIGH, 12000);
  if (duration == 0) return false;


  int distance = duration * 0.0343 / 2;
  return (distance > 0 && distance <= 15);
}


void handleObstacle() {
  stopMotors();
  digitalWrite(BUZZER_PIN, HIGH); // Buzzer turns ON during obstacle
  while (checkObstacle()) {
    delay(50);
  }
  digitalWrite(BUZZER_PIN, LOW);  // Buzzer turns OFF when clear
}


bool checkRFIDThrottled(byte *targetUID) {
  if (millis() - lastRFIDCheck < 60) return false;
  lastRFIDCheck = millis();
  return checkRFID(targetUID);
}


bool checkRFID(byte *targetUID) {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return false;


  bool match = true;
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != targetUID[i]) {
      match = false;
      break;
    }
  }


  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  return match;
}

