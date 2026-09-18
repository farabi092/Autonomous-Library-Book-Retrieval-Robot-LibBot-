#include <SPI.h>
#include <MFRC522.h>


// ==================== CONFIGURATION & TOGGLES ====================
const int BLACK_LINE_STATE = LOW;   // Active-LOW logic (White = 1, Black = 0)
const bool SWAP_IR_SENSORS = false; // Set true if physical left/right IR sensors are swapped


// ==================== PIN DEFINITIONS ====================
#define IN1 5  // Right Motor PWM / Speed
#define IN2 2  // Right Motor Direction
#define IN3 6  // Left Motor PWM / Speed
#define IN4 4  // Left Motor Direction


#define IR_LEFT   A0
#define IR_CENTER A1
#define IR_RIGHT  A2


#define TRIG_PIN 3
#define ECHO_PIN A5
#define BUZZER_PIN 1


#define SS_PIN  A3  // RFID SDA
#define RST_PIN A4  // RFID RST


// ==================== SLOW-SPEED PID PARAMETERS ====================
float Kp = 18.0;  
float Kd = 8.0;  
float Ki = 0.0;  


int BASE_SPEED = 90;  // Reduced cruise speed
int MIN_SPEED  = 50;  // Minimum speed threshold
int MAX_SPEED  = 135; // Maximum speed ceiling


float error = 0, lastError = 0, integral = 0;


// ==================== RFID CONFIGURATION ====================
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


void setup() {
  // Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();


  // Sensors & Buzzer
  pinMode(IR_LEFT, INPUT); pinMode(IR_CENTER, INPUT); pinMode(IR_RIGHT, INPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);


  // RFID Setup
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max); // Boost antenna sensitivity for reading while moving
}


void loop() {
  // Global Librarian Tag Check (Toggle Start / Stop)
  if (checkRFIDThrottled(LIBRARIAN_UID)) {
    if (currentState == WAIT_FOR_START) {
      triggerBuzzer(300);
      currentState = FOLLOW_TO_SHELF;
    } else {
      // Immediate full stop if Librarian tag is tapped while running
      stopMotors();
      doubleBeep();
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
        doubleBeep();
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
        doubleBeep();
        delay(1500); // Brief pause to confirm book stop
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


// ==================== PID LINE FOLLOWING ====================


void followLinePID() {
  int rawL = (digitalRead(IR_LEFT) == BLACK_LINE_STATE) ? 1 : 0;
  int C    = (digitalRead(IR_CENTER) == BLACK_LINE_STATE) ? 1 : 0;
  int rawR = (digitalRead(IR_RIGHT) == BLACK_LINE_STATE) ? 1 : 0;


  int L = SWAP_IR_SENSORS ? rawR : rawL;
  int R = SWAP_IR_SENSORS ? rawL : rawR;


  // Position Error Mapping
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
 
  analogWrite(IN1, 110); digitalWrite(IN2, LOW);
  analogWrite(IN3, 80);  digitalWrite(IN4, HIGH);
 
  delay(650);
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
  digitalWrite(BUZZER_PIN, HIGH);
  while (checkObstacle()) {
    delay(50);
  }
  digitalWrite(BUZZER_PIN, LOW);
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


void triggerBuzzer(int durationMs) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(durationMs);
  digitalWrite(BUZZER_PIN, LOW);
}


void doubleBeep() {
  triggerBuzzer(100);
  delay(80);
  triggerBuzzer(100);
}

