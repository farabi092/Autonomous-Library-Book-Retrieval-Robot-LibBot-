

| \# | Component | Pin / wire | Connect to | Important note |
| ----- | ----- | ----- | ----- | ----- |
| **MOTOR DRIVER — YOUR CURRENT WIRING** |  |  |  |  |
| 1 | L298N | **OUT1** | RIGHT motor wire 1 | Keep as you already connected |
| 2 | L298N | **OUT2** | RIGHT motor wire 2 | Keep as you already connected |
| 3 | L298N | **OUT3** | LEFT motor wire 1 | Connect left motor here |
| 4 | L298N | **OUT4** | LEFT motor wire 2 | Connect left motor here |
| 5 | L298N | **IN1 \- orange** | Arduino **D5** | RIGHT motor forward/speed |
| 6 | L298N | **IN2 \- white** | Arduino **D2** | RIGHT motor reverse |
| 7 | L298N | **IN3 \- white** | Arduino **D6** | LEFT motor forward/speed |
| 8 | L298N | **IN4 \- purple** | Arduino **D4** | LEFT motor reverse |
| 9 | L298N | **ENA jumper** | Keep jumper **ON** | Do not connect ENA to Arduino |
| 10 | L298N | **ENB jumper** | Keep jumper **ON** | Do not connect ENB to Arduino |
| 11 | L298N | **12V/VIN terminal** | Battery positive through main switch | Motor power |
| 12 | L298N | **GND** | Battery − / common GND | Extremely important |
| 13 | L298N | **5V terminal** | **Leave unconnected** | As specified in project plan |
| **IR LINE SENSOR ARRAY** |  |  |  |  |
| 14 | IR array | **VCC / 5V** | Regulated **5V rail** |  |
| 15 | IR array | **GND** | Common GND |  |
| 16 | IR array | **OUT1- blue**  | Arduino **A0** | Left line sensor |
| 17 | IR array | **OUT2**  | Nothing | Leave disconnected |
| 18 | IR array | **OUT3 \- orange** | Arduino **A1** | Centre line sensor |
| 19 | IR array | **OUT4** | Nothing | Leave disconnected |
| 20 | IR array | **OUT5 \-brown** | Arduino **A2** | Right line sensor |
| **HC-SR04 SONAR** |  |  |  |  |
| 21 | HC-SR04 | **VCC** | Regulated **5V rail** |  |
| 22 | HC-SR04 | **GND** | Common GND |  |
| 23 | HC-SR04 | **TRIG \- yellow** | Arduino **D3** |  |
| 24 | HC-SR04 | **ECHO- brown** | Arduino **A5** |  |

**4 Servo connections**

| \# | Component | Pin / wire | Connect to | Important note |
| ----- | ----- | ----- | ----- | ----- |
| 25 | Servo 1 | Signal   –  green | **Arduino D7** | Arm servo 1(Base) |
| 26 | Servo 1 | Red / VCC | **Regulated 5V rail** | NOT Arduino VIN |
| 27 | Servo 1 | Brown/Black / GND | **Common GND rail** |  |
| 28 | Servo 2 | Signal  – blue  | **Arduino D8** | Arm servo 2(above the base) |
| 29 | Servo 2 | Red / VCC | **Regulated 5V rail** |  |
| 30 | Servo 2 | Brown/Black / GND | **Common GND rail** |  |
| 31 | Servo 3 | Signal  – purple | **Arduino D9** | Arm servo 3 |
| 32 | Servo 3 | Red / VCC | **Regulated 5V rail** |  |
| 33 | Servo 3 | Brown/Black / GND | **Common GND rail** |  |
| 34 | Servo 4 | Signal  – white  | **Arduino D10** | Gripper servo |
| 35 | Servo 4 | Red / VCC | **Regulated 5V rail** |  |
| 36 | Servo 4 | Brown/Black / GND | **Common GND rail** |  |

RFID Connections

| RC522 pin | Connect to now | How |
| ----- | ----- | ----- |
| **3.3V** | Arduino **3.3V** | Direct |
| **GND** | Common GND rail | Direct |
| **RST** | Arduino **A4** | Through 1 kΩ \+ 2 kΩ divider |
| **SDA / SS** | Arduino **A3** | Through 1 kΩ \+ 2 kΩ divider |
| **MOSI** | Arduino **D11** | Through 1 kΩ \+ 2 kΩ divider |
| **MISO** | Arduino **D12** | Direct |
| **SCK** | Arduino **D13** | Through 1 kΩ \+ 2 kΩ divider |
| **IRQ** | Nothing | Leave disconnected |

