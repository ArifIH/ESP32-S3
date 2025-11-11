#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Servo.h>

#define LED1_PIN 1
#define LED2_PIN 2
#define LED3_PIN 39
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BUZZER_PIN 42
#define STEP_PIN 13
#define DIR_PIN 14
#define STEPS_PER_REV 200
#define SERVO_PIN 41
#define BUTTON1_PIN 7
#define BUTTON2_PIN 6
#define POT_PIN 5
#define CLK_PIN 35
#define DT_PIN 36

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SemaphoreHandle_t i2cMutex;
Servo myServo;

volatile int potValue = 0;
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile int lastStateCLK = LOW;
volatile int direction = 0;

SemaphoreHandle_t setupCompleteSemaphore;

void ledBlinkTask(void * pvParameters) {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  for (;;) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void buzzerTask(void * pvParameters) {
  pinMode(BUZZER_PIN, OUTPUT);
  for (;;) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void oledTask(void * pvParameters) {
  String coreID = String(xPortGetCoreID());
  for (;;) {
    if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 0);
      display.println("Integrasi 8-Periferal");
      display.setTextSize(2);
      display.setCursor(0, 20);
      display.print("CORE: ");
      display.println(coreID);
      display.display();
      xSemaphoreGive(i2cMutex);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void stepperTask(void * pvParameters) {
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  for (;;) {
    digitalWrite(DIR_PIN, HIGH);
    for (int i = 0; i < STEPS_PER_REV; i++) {
      digitalWrite(STEP_PIN, HIGH); delayMicroseconds(500);
      digitalWrite(STEP_PIN, LOW); delayMicroseconds(500);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    digitalWrite(DIR_PIN, LOW);
    for (int i = 0; i < STEPS_PER_REV; i++) {
      digitalWrite(STEP_PIN, HIGH); delayMicroseconds(500);
      digitalWrite(STEP_PIN, LOW); delayMicroseconds(500);
    }
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void servoTask(void * pvParameters) {
  myServo.attach(SERVO_PIN);
  for (;;) {
    myServo.write(0);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
    myServo.write(180);
    vTaskDelay(1500 / portTICK_PERIOD_MS);
  }
}

void readButton1Task(void *pvParameters) {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  for (;;) {
    bool state = digitalRead(BUTTON1_PIN) == LOW;
    if (state != button1Pressed) {
      button1Pressed = state;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void readButton2Task(void *pvParameters) {
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  for (;;) {
    bool state = digitalRead(BUTTON2_PIN) == LOW;
    if (state != button2Pressed) {
      button2Pressed = state;
    }
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void readPotTask(void *pvParameters) {
  pinMode(POT_PIN, INPUT);
  for (;;) {
    potValue = analogRead(POT_PIN);
    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

void readEncoderTask(void *pvParameters) {
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK_PIN);
  for (;;) {
    int currentStateCLK = digitalRead(CLK_PIN);
    if (currentStateCLK != lastStateCLK) {
      if (digitalRead(DT_PIN) != currentStateCLK) {
        direction = 1;
      } else {
        direction = -1;
      }
    }
    lastStateCLK = currentStateCLK;
    vTaskDelay(2 / portTICK_PERIOD_MS);
  }
}

void loggingTask(void * pvParameters) {
  xSemaphoreTake(setupCompleteSemaphore, portMAX_DELAY);

  for (;;) {
    int currentPot = potValue;
    bool btn1 = button1Pressed;
    bool btn2 = button2Pressed;
    int encDir = direction;

    Serial.printf("%-10d| %-6s| %-6s| %-12d\n",
                  currentPot,
                  btn1 ? "ON" : "OFF",
                  btn2 ? "ON" : "OFF",
                  encDir);

    direction = 0;
    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  Serial.println("===== MEMULAI PROGRAM INTEGRASI 8-PERIFERAL =====");

  setupCompleteSemaphore = xSemaphoreCreateBinary();

  Serial.println("Inisialisasi Mutex & OLED...");
  i2cMutex = xSemaphoreCreateMutex();
  if (xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE) {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
      Serial.println(F("Alokasi SSD1306 gagal"));
    } else {
      Serial.println("OLED SSD1306 ditemukan!");
      display.clearDisplay();
      display.display();
    }
    xSemaphoreGive(i2cMutex);
  }

  Serial.println("Membuat semua task...");
  xTaskCreatePinnedToCore(readEncoderTask, "EncoderReader", 2048, NULL, 3, NULL, 0);
  xTaskCreatePinnedToCore(readButton1Task, "Button1", 2048, NULL, 2, NULL, 0);
  xTaskCreatePinnedToCore(readPotTask, "ReadPot", 2048, NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(stepperTask, "Stepper", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(readButton2Task, "Button2", 2048, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(ledBlinkTask, "LEDs", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(buzzerTask, "Buzzer", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(oledTask, "OLED", 4096, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(servoTask, "Servo", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(loggingTask, "Logging", 4096, NULL, 1, NULL, 1);

  Serial.println("\n--- Status Pembagian Core ---");
  Serial.println("Core | Prioritas | Task");
  Serial.println("-----|-----------|-----------------");
  Serial.println("  0  |     3     | readEncoderTask");
  Serial.println("  0  |     2     | readButton1Task");
  Serial.println("  0  |     1     | readPotTask");
  Serial.println("  1  |     2     | stepperTask");
  Serial.println("  1  |     2     | readButton2Task");
  Serial.println("  1  |     1     | ledBlinkTask");
  Serial.println("  1  |     1     | buzzerTask");
  Serial.println("  1  |     1     | oledTask");
  Serial.println("  1  |     1     | servoTask");
  Serial.println("  1  |     1     | loggingTask");
  Serial.println("---------------------------------");
  Serial.println("===== Inisialisasi Selesai. =====");

  Serial.println("\n--- Status Periferal (Real-time) ---");
  Serial.println("Pot(4095) | Btn 1 | Btn 2 | Encoder Dir");
  Serial.println("----------|-------|-------|-------------");

  xSemaphoreGive(setupCompleteSemaphore);
}

void loop() {
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}