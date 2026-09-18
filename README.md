# Autonomous Library Book Retrieval Robot (LibBot)

An autonomous robot that navigates to bookshelves, picks up books using a robotic arm, and returns them using line-following technology and RFID detection.

## Features
- ✅ Line following with PID control
- ✅ RFID card detection (Librarian, Shelf, Book)
- ✅ Obstacle detection with ultrasonic sensor
- ✅ 4-servo robotic arm with gripper
- ✅ Active-LOW IR sensor support

## Hardware Components

### Microcontroller
- Arduino Uno

### Motors & Drivers
- 2x DC Motors (with wheels)
- 1x Motor Driver Module (L298N)

### Sensors
- 3x IR Sensors (line detection)
- 1x Ultrasonic Sensor (HC-SR04)
- 1x RFID Reader (MFRC522)

### Actuators
- 4x SG90 Servo Motors (Base, Mid Joint, Upper Joint, Gripper)

### Power
- External 5V/2A Power Supply
- 9V Battery for Arduino (optional)

### RFID Cards
- Librarian UID: `0xA3 0x3D 0x3D 0xFC`
- Shelf UID: `0x33 0x45 0x3F 0xFC`
- Book UID: `0x49 0xDB 0x07 0x05`

## Pin Connections

### Motors
| Pin | Component | Purpose |
|-----|-----------|---------|
| 5 | IN1 | Right Motor PWM |
| 2 | IN2 | Right Motor Direction |
| 6 | IN3 | Left Motor PWM |
| 4 | IN4 | Left Motor Direction |

### Sensors
| Pin | Component |
|-----|-----------|
| A0 | IR Left |
| A1 | IR Center |
| A2 | IR Right |
| 3 | Ultrasonic Trigger |
| A5 | Ultrasonic Echo |

### RFID
| Pin | Component |
|-----|-----------|
| A3 | RFID SDA |
| A4 | RFID RST |
| 11 | MOSI |
| 12 | MISO |
| 13 | SCK |

### Servos
| Pin | Servo |
|-----|-------|
| 7 | Base |
| 8 | Mid Joint |
| 9 | Upper Joint |
| 10 | Gripper |

### Buzzer
| Pin | Component |
|-----|-----------|
| 1 (TX) | Buzzer |

## How to Use

### 1. Upload Code
- Open `code/complete_robot_code.ino` in Arduino IDE
- Select Board: Arduino Uno
- Select Port: COM3 (or your port)
- Click Upload

### 2. Calibrate Servos
- Use `code/arm_gripper_only.ino` to test servo angles
- Send commands via Serial Monitor: `GRAB`, `REST`, `OPEN`, `CLOSE`
- Update angle values in main code

### 3. Run Robot
- Tap Librarian card to start
- Robot follows black line to shelf
- Shelf card detected → Robot stops 2 seconds, turns left
- Robot continues following line to book
- Book card detected → Arm grabs book
- Robot follows line with book
- Tap Librarian card to stop

## Workflow
