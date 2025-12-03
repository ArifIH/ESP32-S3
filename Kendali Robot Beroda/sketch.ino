#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Konfigurasi OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definisi
#define BTN_MAJU 7
#define BTN_MUNDUR 6
#define BTN_BERHENTI 5
#define ENCODER_CLK 12
#define ENCODER_DT 11
#define ENCODER_SW 10
#define STEP_PIN 14
#define DIR_PIN 13
#define LED_PIN 16

// KONFIGURASI MOTOR UNTUK WOKWI (Animasi Terlihat)
#define STEPS_PER_CLICK 400      // Banyak steps agar rotasi terlihat jelas
#define STEP_DELAY_US 2000       // Delay lambat agar animasi smooth (2ms per step)
#define CONTINUOUS_MODE true     // Mode kontinyu saat tombol ditekan

// KONFIGURASI ENCODER SIMULASI
// Motor Stepper: 200 steps/rev
// Encoder KY-040: 20 clicks/rev
// Ratio: 200 ÷ 20 = 10 steps per encoder click
#define MOTOR_STEPS_PER_REV 200     // Stepper motor standard
#define ENCODER_CLICKS_PER_REV 20   // KY-040 encoder
#define STEPS_PER_ENCODER_CLICK (MOTOR_STEPS_PER_REV / ENCODER_CLICKS_PER_REV)  // = 10
#define ENABLE_AUTO_ENCODER true    // Auto-update encoder dari motor

// Variabel Global
volatile int encoderPos = 0;
volatile int lastEncoderCLK = 0;
volatile long motorSteps = 0;              // Mulai dari 0, tidak bergerak
volatile int buttonPressCount = 0;
volatile int simulatedEncoderPos = 0;      // Encoder juga mulai dari 0

// Queue dan Mutex
QueueHandle_t commandQueue;
SemaphoreHandle_t motorMutex;
SemaphoreHandle_t encoderMutex;

// Enum untuk perintah gerakan
enum MotorCommand {
  CMD_STOP = 0,
  CMD_FORWARD = 1,
  CMD_BACKWARD = 2
};

// Variabel status
volatile MotorCommand currentCommand = CMD_STOP;
volatile bool motorRunning = false;
volatile bool keepRunning = false;

// ISR untuk Encoder (input manual dari user)
void IRAM_ATTR encoderISR() {
  int currentCLK = digitalRead(ENCODER_CLK);
  int currentDT = digitalRead(ENCODER_DT);
  
  if (currentCLK != lastEncoderCLK) {
    if (xSemaphoreTakeFromISR(encoderMutex, NULL) == pdTRUE) {
      if (currentDT != currentCLK) {
        encoderPos++;
      } else {
        encoderPos--;
      }
      xSemaphoreGiveFromISR(encoderMutex, NULL);
    }
  }
  lastEncoderCLK = currentCLK;
}

// Fungsi untuk update simulated encoder dari motor steps
void updateSimulatedEncoder() {
  if (ENABLE_AUTO_ENCODER) {
    if (xSemaphoreTake(encoderMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      simulatedEncoderPos = motorSteps / STEPS_PER_ENCODER_CLICK;
      xSemaphoreGive(encoderMutex);
    }
  }
}

// ==================== TASKS ====================

// Task untuk membaca button
void buttonTask(void *parameter) {
  MotorCommand cmd;
  bool wasPressed[3] = {false, false, false};
  
  Serial.println("[BUTTON TASK] Started on Core " + String(xPortGetCoreID()));
  
  while (1) {
    bool btn1 = (digitalRead(BTN_MAJU) == LOW);
    bool btn2 = (digitalRead(BTN_MUNDUR) == LOW);
    bool btn3 = (digitalRead(BTN_BERHENTI) == LOW);
    
    // Button 1 - MAJU (dengan mode continuous)
    if (btn1 && !wasPressed[0]) {
      wasPressed[0] = true;
      cmd = CMD_FORWARD;
      buttonPressCount++;
      keepRunning = CONTINUOUS_MODE;
      
      Serial.println("\n>>> FORWARD Button Pressed <<<");
      
      if (xQueueSend(commandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
        Serial.println("[QUEUE] Forward command sent");
        digitalWrite(LED_PIN, HIGH);
      }
    } else if (!btn1) {
      wasPressed[0] = false;
      if (currentCommand == CMD_FORWARD) {
        keepRunning = false;
      }
    }
    
    // Button 2 - MUNDUR (dengan mode continuous)
    if (btn2 && !wasPressed[1]) {
      wasPressed[1] = true;
      cmd = CMD_BACKWARD;
      buttonPressCount++;
      keepRunning = CONTINUOUS_MODE;
      
      Serial.println("\n>>> BACKWARD Button Pressed <<<");
      
      if (xQueueSend(commandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
        Serial.println("[QUEUE] Backward command sent");
        digitalWrite(LED_PIN, HIGH);
      }
    } else if (!btn2) {
      wasPressed[1] = false;
      if (currentCommand == CMD_BACKWARD) {
        keepRunning = false;
      }
    }
    
    // Button 3 - STOP
    if (btn3 && !wasPressed[2]) {
      wasPressed[2] = true;
      cmd = CMD_STOP;
      buttonPressCount++;
      keepRunning = false;
      
      Serial.println("\n>>> STOP Button Pressed <<<");
      
      if (xQueueSend(commandQueue, &cmd, portMAX_DELAY) == pdTRUE) {
        Serial.println("[QUEUE] Stop command sent");
        digitalWrite(LED_PIN, LOW);
      }
    } else if (!btn3) {
      wasPressed[2] = false;
    }
    
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// Task untuk encoder
void encoderTask(void *parameter) {
  Serial.println("[ENCODER TASK] Started on Core " + String(xPortGetCoreID()));
  int lastManualPos = 0;
  int lastSimulatedPos = 0;
  
  while (1) {
    if (xSemaphoreTake(encoderMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      int currentManual = encoderPos;
      int currentSimulated = simulatedEncoderPos;
      xSemaphoreGive(encoderMutex);
      
      // Report manual encoder changes
      if (currentManual != lastManualPos) {
        Serial.println("[ENCODER] Manual Position: " + String(currentManual));
        lastManualPos = currentManual;
      }
      
      // Report simulated encoder changes
      if (currentSimulated != lastSimulatedPos) {
        Serial.println("[ENCODER] Simulated Position: " + String(currentSimulated) + 
                      " (from " + String(motorSteps) + " motor steps)");
        lastSimulatedPos = currentSimulated;
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// Fungsi untuk step motor (dipanggil berulang untuk continuous mode)
void stepMotor(bool forward, int steps) {
  digitalWrite(DIR_PIN, forward ? HIGH : LOW);
  
  long startSteps = motorSteps;  // Catat posisi awal
  int actualSteps = 0;            // Counter steps yang benar-benar jalan
  
  for (int i = 0; i < steps; i++) {
    // Cek apakah masih harus jalan (untuk mode continuous)
    if (!keepRunning && CONTINUOUS_MODE) {
      Serial.println("[MOTOR] Stopped by user at step " + String(i) + "/" + String(steps));
      break;
    }
    
    // Generate step pulse
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(STEP_DELAY_US);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(STEP_DELAY_US);
    
    // Update counter
    if (forward) {
      motorSteps++;
    } else {
      motorSteps--;
    }
    actualSteps++;
    
    // Update simulated encoder setiap beberapa steps
    if (i % 10 == 0) {
      updateSimulatedEncoder();
    }
    
    // Yield to other tasks periodically
    if (i % 20 == 0) {
      vTaskDelay(1);
    }
  }
  
  // Final encoder update setelah gerakan selesai
  updateSimulatedEncoder();
  
  // Verifikasi
  long endSteps = motorSteps;
  long deltaSteps = endSteps - startSteps;
  if (abs(deltaSteps) != actualSteps) {
    Serial.println("[WARNING] Step mismatch! Expected: " + String(actualSteps) + 
                  ", Actual delta: " + String(deltaSteps));
  }
}

// Task untuk motor control
void motorControlTask(void *parameter) {
  MotorCommand receivedCmd;
  int commandCount = 0;
  
  Serial.println("[MOTOR TASK] Started on Core " + String(xPortGetCoreID()));
  Serial.println("[MOTOR] Step delay: " + String(STEP_DELAY_US) + "us");
  Serial.println("[MOTOR] Steps per command: " + String(STEPS_PER_CLICK));
  Serial.println("[MOTOR] Continuous mode: " + String(CONTINUOUS_MODE ? "ON" : "OFF"));
  Serial.println("[MOTOR] Auto-encoder: " + String(ENABLE_AUTO_ENCODER ? "ON" : "OFF"));
  Serial.println("[MOTOR] Encoder ratio: 1 click per " + String(STEPS_PER_ENCODER_CLICK) + " steps");
  
  while (1) {
    // Cek queue
    if (xQueueReceive(commandQueue, &receivedCmd, pdMS_TO_TICKS(100)) == pdTRUE) {
      
      commandCount++;
      
      Serial.println("\n========================================");
      Serial.println("[MOTOR] Command #" + String(commandCount) + " received");
      Serial.println("========================================");
      
      // Update current command dengan mutex
      if (xSemaphoreTake(motorMutex, portMAX_DELAY) == pdTRUE) {
        currentCommand = receivedCmd;
        xSemaphoreGive(motorMutex);
      }
      
      // Eksekusi motor
      switch (receivedCmd) {
        case CMD_FORWARD:
          Serial.println("[MOTOR] ▶▶▶ MOVING FORWARD ▶▶▶");
          motorRunning = true;
          
          if (CONTINUOUS_MODE) {
            // Mode kontinyu - jalan terus selama tombol ditekan
            Serial.println("[MOTOR] Continuous forward mode - hold button to keep moving");
            long startPos = motorSteps;
            int iterations = 0;
            
            while (keepRunning && currentCommand == CMD_FORWARD) {
              stepMotor(true, 50); // Step sedikit-sedikit
              iterations++;
            }
            
            long endPos = motorSteps;
            Serial.println("[MOTOR] Continuous forward stopped");
            Serial.println("[MOTOR] Total iterations: " + String(iterations));
            Serial.println("[MOTOR] Steps moved: " + String(endPos - startPos));
          } else {
            // Mode sekali klik
            long beforeSteps = motorSteps;
            stepMotor(true, STEPS_PER_CLICK);
            long afterSteps = motorSteps;
            Serial.println("[MOTOR] Forward complete: " + String(STEPS_PER_CLICK) + " steps commanded");
            Serial.println("[MOTOR] Actual steps moved: " + String(afterSteps - beforeSteps));
          }
          
          motorRunning = false;
          break;
          
        case CMD_BACKWARD:
          Serial.println("[MOTOR] ◀◀◀ MOVING BACKWARD ◀◀◀");
          motorRunning = true;
          
          if (CONTINUOUS_MODE) {
            // Mode kontinyu - jalan terus selama tombol ditekan
            Serial.println("[MOTOR] Continuous backward mode - hold button to keep moving");
            long startPos = motorSteps;
            int iterations = 0;
            
            while (keepRunning && currentCommand == CMD_BACKWARD) {
              stepMotor(false, 50); // Step sedikit-sedikit
              iterations++;
            }
            
            long endPos = motorSteps;
            Serial.println("[MOTOR] Continuous backward stopped");
            Serial.println("[MOTOR] Total iterations: " + String(iterations));
            Serial.println("[MOTOR] Steps moved: " + String(endPos - startPos));
          } else {
            // Mode sekali klik
            long beforeSteps = motorSteps;
            stepMotor(false, STEPS_PER_CLICK);
            long afterSteps = motorSteps;
            Serial.println("[MOTOR] Backward complete: " + String(STEPS_PER_CLICK) + " steps commanded");
            Serial.println("[MOTOR] Actual steps moved: " + String(afterSteps - beforeSteps));
          }
          
          motorRunning = false;
          break;
          
        case CMD_STOP:
          Serial.println("[MOTOR] ■ STOPPED ■");
          motorRunning = false;
          keepRunning = false;
          break;
      }
      
      Serial.println("[MOTOR] Total steps: " + String(motorSteps));
      Serial.println("[MOTOR] Simulated encoder: " + String(simulatedEncoderPos));
      Serial.println("========================================\n");
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// Task untuk display
void displayTask(void *parameter) {
  Serial.println("[DISPLAY TASK] Started on Core " + String(xPortGetCoreID()));
  
  while (1) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.println("Robot Stepper");
    display.println("==============");
    
    // Status command
    display.print("Status: ");
    if (xSemaphoreTake(motorMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      switch (currentCommand) {
        case CMD_FORWARD:
          display.println(motorRunning ? ">>> MAJU" : "MAJU");
          break;
        case CMD_BACKWARD:
          display.println(motorRunning ? "<<< MUNDUR" : "MUNDUR");
          break;
        case CMD_STOP:
          display.println("[ STOP ]");
          break;
      }
      xSemaphoreGive(motorMutex);
    }
    
    // Motor info
    display.print("Motor: ");
    display.println(motorRunning ? "RUNNING" : "IDLE");
    
    // Steps info dengan detail
    display.print("Steps: ");
    display.print(motorSteps);
    
    // Hitung putaran
    float rotations = (float)motorSteps / MOTOR_STEPS_PER_REV;
    display.print(" (");
    display.print(rotations, 1);
    display.println("rev)");
    
    // Encoder info (manual dan simulated)
    if (xSemaphoreTake(encoderMutex, pdMS_TO_TICKS(5)) == pdTRUE) {
      if (ENABLE_AUTO_ENCODER) {
        display.print("Enc(Auto): ");
        display.println(simulatedEncoderPos);
      } else {
        display.print("Encoder: ");
        display.println(encoderPos);
      }
      xSemaphoreGive(encoderMutex);
    }
    
    // Button presses
    display.print("BtnPress: ");
    display.println(buttonPressCount);
    
    // Progress bar jika motor running
    if (motorRunning) {
      display.fillRect(0, 56, 128, 8, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
      display.setCursor(35, 56);
      display.print("MOVING");
    }
    
    display.display();
    
    vTaskDelay(pdMS_TO_TICKS(100)); // Update lebih cepat untuk animasi
  }
}

// ==================== SETUP ====================

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("\n\n");
  Serial.println("╔════════════════════════════════════════╗");
  Serial.println("║   ROBOT STEPPER CONTROL - WOKWI        ║");
  Serial.println("║   With Auto-Encoder Simulation         ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println();
  
  // Pin initialization
  pinMode(BTN_MAJU, INPUT_PULLUP);
  pinMode(BTN_MUNDUR, INPUT_PULLUP);
  pinMode(BTN_BERHENTI, INPUT_PULLUP);
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);
  pinMode(ENCODER_SW, INPUT_PULLUP);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(STEP_PIN, LOW);
  digitalWrite(DIR_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("[INIT] All pins configured");
  Serial.println("  Motor: STEP=" + String(STEP_PIN) + ", DIR=" + String(DIR_PIN));
  Serial.println("  Buttons: MAJU=" + String(BTN_MAJU) + ", MUNDUR=" + String(BTN_MUNDUR) + ", STOP=" + String(BTN_BERHENTI));
  
  // REMOVED: Motor test animation - motor stays at 0
  Serial.println("\n[INFO] Motor test disabled - motor will stay at position 0");
  Serial.println("[INFO] Press buttons to start motor movement");
  
  // I2C and OLED
  Wire.begin(8, 9);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("[ERROR] OLED init failed!");
    while(1) delay(1000);
  }
  Serial.println("[INIT] OLED ready");
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("STEPPER");
  display.setCursor(10, 30);
  display.println("CONTROL");
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println("Initializing...");
  display.display();
  delay(1000);
  
  // Create mutex
  motorMutex = xSemaphoreCreateMutex();
  encoderMutex = xSemaphoreCreateMutex();
  commandQueue = xQueueCreate(10, sizeof(MotorCommand));
  
  if (motorMutex == NULL || encoderMutex == NULL || commandQueue == NULL) {
    Serial.println("[ERROR] Mutex/Queue creation failed!");
    while(1) delay(1000);
  }
  Serial.println("[INIT] Mutex and Queue created");
  
  // Encoder interrupt
  lastEncoderCLK = digitalRead(ENCODER_CLK);
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), encoderISR, CHANGE);
  Serial.println("[INIT] Encoder interrupt attached");
  
  // Create tasks
  Serial.println("\n[INIT] Creating tasks...");
  
  xTaskCreatePinnedToCore(buttonTask, "ButtonTask", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(encoderTask, "EncoderTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(motorControlTask, "MotorTask", 8192, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(displayTask, "DisplayTask", 4096, NULL, 1, NULL, 0);
  
  Serial.println("[INIT] All tasks created");
  
  delay(1000);
  
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║           SYSTEM READY!                ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.println("\n📋 CONTROLS:");
  Serial.println("  🟢 BTN1 (GPIO7)  = Forward");
  Serial.println("  🟢 BTN2 (GPIO6)  = Backward");
  Serial.println("  🔵 BTN3 (GPIO5)  = Stop");
  
  if (CONTINUOUS_MODE) {
    Serial.println("\n⚙️  MODE: Continuous");
    Serial.println("   Hold button to keep motor running");
    Serial.println("   Release button to stop");
  } else {
    Serial.println("\n⚙️  MODE: Single Click");
    Serial.println("   Each click = " + String(STEPS_PER_CLICK) + " steps");
  }
  
  Serial.println("\n🔄 ENCODER:");
  Serial.println("   Auto-update: " + String(ENABLE_AUTO_ENCODER ? "ENABLED" : "DISABLED"));
  Serial.println("   Motor: " + String(MOTOR_STEPS_PER_REV) + " steps/rev");
  Serial.println("   Encoder: " + String(ENCODER_CLICKS_PER_REV) + " clicks/rev");
  Serial.println("   Ratio: " + String(STEPS_PER_ENCODER_CLICK) + " steps = 1 encoder click");
  Serial.println("   You can also rotate encoder manually!");
  
  Serial.println("\n⏱️  TIMING:");
  Serial.println("   Step delay: " + String(STEP_DELAY_US) + " microseconds");
  Serial.println("   Speed: " + String(1000000.0 / (STEP_DELAY_US * 2.0)) + " steps/sec");
  Serial.println("\n▶️  Press buttons to start...\n");
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20);
  display.println("READY!");
  display.setTextSize(1);
  display.setCursor(15, 45);
  display.println("Press button");
  display.display();
  delay(2000);
}

void loop() {
  static unsigned long lastStatus = 0;
  
  if (millis() - lastStatus > 10000) {
    Serial.println("\n📊 STATUS:");
    Serial.println("  Heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("  Button presses: " + String(buttonPressCount));
    Serial.println("  Motor steps: " + String(motorSteps));
    
    if (xSemaphoreTake(encoderMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
      Serial.println("  Manual Encoder: " + String(encoderPos));
      Serial.println("  Simulated Encoder: " + String(simulatedEncoderPos));
      xSemaphoreGive(encoderMutex);
    }
    
    Serial.println();
    lastStatus = millis();
  }
  
  delay(100);
}