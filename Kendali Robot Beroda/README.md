# Stepper Motor Control with Encoder Simulation

## Description
This project is a stepper motor control system implemented using an ESP32 (or similar microcontroller). It simulates encoder feedback based on motor steps and allows controlling the motor using physical buttons. The system supports continuous movement and step-by-step movement, with real-time display updates and encoder simulation.

The system uses:
- **Encoder (KY-040)** for manual input.  
- **Stepper Motor** for movement, with forward and backward control.  
- **OLED Display (SSD1306)** to show the current motor status, encoder position, and other system information.  
- **Button inputs** for controlling the motor (forward, backward, stop).  

### Features:
- Real-time motor control via physical buttons.  
- Simulated encoder position based on motor steps.  
- OLED display shows the current motor status, steps, and encoder positions.  
- Continuous mode and step mode for motor movement.  
- Supports multiple tasks using FreeRTOS: Button task, Encoder task, Motor task, Display task.  
- Automatic encoder simulation and manual encoder feedback.  
- Queue‑based communication for task synchronization.  

---

## Requirements

- **Hardware:**  
  - ESP32 or similar microcontroller.  
  - Stepper motor driver (e.g., A4988 or DRV8825).  
  - KY-040 rotary encoder.  
  - OLED display (SSD1306).  
  - 3 buttons for motor control (forward, backward, stop).  
  - LEDs (optional, for motor activity indication).  

- **Libraries:**  
  - `Wire.h` for I2C communication.  
  - `Adafruit_GFX.h` and `Adafruit_SSD1306.h` for OLED display control.  
  - FreeRTOS for task management and synchronization.  

---

## Wiring Diagram

- **Stepper Motor:**  
  - `STEP_PIN`: Connected to the step pin of the stepper driver.  
  - `DIR_PIN`: Connected to the direction pin of the stepper driver.  

- **Encoder (KY-040):**  
  - `ENCODER_CLK`: Connected to the CLK pin of the encoder.  
  - `ENCODER_DT`: Connected to the DT pin of the encoder.  
  - `ENCODER_SW`: Connected to the switch pin of the encoder.  

- **Buttons:**  
  - `BTN_MAJU`: Connected to GPIO7 (Forward).  
  - `BTN_MUNDUR`: Connected to GPIO6 (Backward).  
  - `BTN_BERHENTI`: Connected to GPIO5 (Stop).  

- **LED:**  
  - `LED_PIN`: Connected to GPIO16 to indicate motor movement.  

- **OLED Display:**  
  - Use the I2C interface (SCL, SDA).  

---

## Wokwi Simulation
- _Simulation_: [Wokwi](https://wokwi.com/projects/449098490499264513)

---

## Pin Definitions
```cpp
#define BTN_MAJU 7         // Button for moving forward  
#define BTN_MUNDUR 6       // Button for moving backward  
#define BTN_BERHENTI 5     // Button for stopping the motor  
#define ENCODER_CLK 12     // Encoder clock pin  
#define ENCODER_DT 11      // Encoder data pin  
#define ENCODER_SW 10      // Encoder switch pin  
#define STEP_PIN 14        // Step pin for stepper motor  
#define DIR_PIN 13         // Direction pin for stepper motor  
#define LED_PIN 16         // LED pin to indicate motor status  
