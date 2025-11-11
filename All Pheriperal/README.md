# 8-Peripheral Integration Project with ESP32

## Project Overview
This project integrates 8 different peripherals with an ESP32 microcontroller, leveraging FreeRTOS tasks for real-time multitasking. The code manages an OLED display, LED control, buzzer, stepper motor, servo, buttons, potentiometer, and rotary encoder. Each peripheral is handled by a dedicated task, demonstrating how to interact with different hardware components simultaneously.

## Hardware Pin Configuration
Below is the list of hardware pins used in the project:

- **LED Pins**:
  - `LED1_PIN`: GPIO 1
  - `LED2_PIN`: GPIO 2
  - `LED3_PIN`: GPIO 39
  
- **Buzzer Pin**:
  - `BUZZER_PIN`: GPIO 42

- **Stepper Motor Pins**:
  - `STEP_PIN`: GPIO 13 (Step)
  - `DIR_PIN`: GPIO 14 (Direction)
  - `STEPS_PER_REV`: 200 (The number of steps for a full revolution)

- **Servo Motor Pin**:
  - `SERVO_PIN`: GPIO 41

- **Button Pins**:
  - `BUTTON1_PIN`: GPIO 7
  - `BUTTON2_PIN`: GPIO 6
  
- **Potentiometer Pin**:
  - `POT_PIN`: GPIO 5 (Analog input)

- **Rotary Encoder Pins**:
  - `CLK_PIN`: GPIO 35 (Clock)
  - `DT_PIN`: GPIO 36 (Data)

## Library Requirements
This project uses several libraries for hardware and task management. Make sure the following libraries are installed in your Arduino IDE:

1. **Wire**: For I2C communication with the OLED display.
   - _Library Link_: [Wire - Arduino](https://www.arduino.cc/en/Reference/Wire)
   
2. **Adafruit_GFX**: Core graphics library for Adafruit displays.
   - _Library Link_: [Adafruit_GFX - GitHub](https://github.com/adafruit/Adafruit-GFX-Library)
   
3. **Adafruit_SSD1306**: OLED display library for the SSD1306 display module.
   - _Library Link_: [Adafruit_SSD1306 - GitHub](https://github.com/adafruit/Adafruit_SSD1306)
   
4. **ESP32Servo**: For controlling servo motors with the ESP32.
   - _Library Link_: [ESP32Servo - GitHub](https://github.com/jlemke/ESP32Servo)

## Task Overview
- **LED Blink Task**: Blinks three LEDs (`LED1_PIN`, `LED2_PIN`, `LED3_PIN`) on and off every 1 second.
  
- **Buzzer Task**: Toggles a buzzer on and off every 1 second using `BUZZER_PIN`.
  
- **OLED Display Task**: Displays information on a 128x64 OLED screen (`OLED_RESET = -1`). This includes showing the core ID of the ESP32, updated every second.
  
- **Stepper Motor Task**: Controls a stepper motor using `STEP_PIN` and `DIR_PIN`. It rotates the motor forward and backward with a delay of 2 seconds after completing a full rotation.
  
- **Servo Task**: Moves a servo motor (`SERVO_PIN`) between 0° and 180° with a delay of 1.5 seconds.
  
- **Button 1 and Button 2 Tasks**: Reads the state of two buttons (`BUTTON1_PIN` and `BUTTON2_PIN`) and stores their state (pressed or not).
  
- **Potentiometer Task**: Reads the analog value from a potentiometer connected to `POT_PIN`.
  
- **Rotary Encoder Task**: Reads the rotary encoder’s direction using `CLK_PIN` (Clock) and `DT_PIN` (Data). Determines the direction of rotation (clockwise or counterclockwise).

- **Logging Task**: Logs values from the potentiometer, button states, and rotary encoder direction to the serial monitor every 250 ms.

## Setup Instructions

1. **Hardware Setup**:
   - Connect the components as per the pin configuration mentioned above.
   - Ensure the stepper motor, servo, and rotary encoder are properly connected to their respective pins.

2. **Library Installation**:
   - In the Arduino IDE, go to `Sketch` -> `Include Library` -> `Manage Libraries`.
   - Search and install the following libraries:
     - **Wire**
     - **Adafruit GFX**
     - **Adafruit SSD1306**
     - **ESP32Servo**

3. **Upload Code**:
   - Select the appropriate board from the Arduino IDE (`ESP32 Dev Module`).
   - Upload the code to the ESP32.

## Pin and Task Summary

| Peripheral        | Pin         | Task                        | Description                                            |
|-------------------|-------------|-----------------------------|--------------------------------------------------------|
| **LEDs**          | GPIO 1, 2, 39 | `ledBlinkTask`               | Blinks LEDs on and off.                               |
| **Buzzer**        | GPIO 42      | `buzzerTask`                 | Turns the buzzer on and off.                          |
| **OLED Display**  | I2C Pins (SDA, SCL) | `oledTask`                  | Displays info on OLED screen.                         |
| **Stepper Motor** | GPIO 13, 14  | `stepperTask`                | Controls stepper motor rotation.                      |
| **Servo**         | GPIO 41      | `servoTask`                  | Moves servo between 0° and 180°.                      |
| **Button 1**      | GPIO 7       | `readButton1Task`            | Reads button 1 state.                                 |
| **Button 2**      | GPIO 6       | `readButton2Task`            | Reads button 2 state.                                 |
| **Potentiometer** | GPIO 5       | `readPotTask`                | Reads potentiometer value.                             |
| **Rotary Encoder**| GPIO 35, 36  | `readEncoderTask`            | Reads rotary encoder direction.                       |

## Task and Core Assignment
This project uses **FreeRTOS** tasks to assign different operations to each core of the ESP32:

- **Core 0**:
  - `readEncoderTask`
  - `readButton1Task`
  - `readPotTask`
  
- **Core 1**:
  - `stepperTask`
  - `readButton2Task`
  - `ledBlinkTask`
  - `buzzerTask`
  - `oledTask`
  - `servoTask`
  - `loggingTask`

## Vidio Output Example
- _Vidio&Documentation_: [GDRIVE](https://drive.google.com/drive/folders/1hyCD7lrclkVvjyCe3S9Difls5EJE2t-j?usp=sharing)
