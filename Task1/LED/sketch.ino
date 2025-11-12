#include <Arduino.h>

#define LED1_PIN 40
#define LED2_PIN 0
#define LED3_PIN 19
#define LED4_PIN 15
#define LED5_PIN 54
#define LED6_PIN 14

#define CORE_0 0
#define CORE_1 1

// Task untuk Core 0: LED 1, 2, 42 berkedip bersama
void ledGroup1Task(void * pvParameters) {
  Serial.print("ledGroup1Task berjalan di Core: ");
  Serial.println(xPortGetCoreID());

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  for (;;) {
    Serial.println("Core " + String(xPortGetCoreID()) + ": Group 1 LEDs ON (PIN 1,2,42)");
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    digitalWrite(LED3_PIN, HIGH);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    Serial.println("Core " + String(xPortGetCoreID()) + ": Group 1 LEDs OFF (PIN 1,2,42)");
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    digitalWrite(LED3_PIN, LOW);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Task untuk Core 1: LED 4, 5, 6 berkedip bergantian
void ledGroup2Task(void * pvParameters) {
  Serial.print("ledGroup2Task berjalan di Core: ");
  Serial.println(xPortGetCoreID());

  // Array untuk LED group 2 (PIN 4,5,6)
  int ledPins[] = {LED4_PIN, LED5_PIN, LED6_PIN};
  int ledCount = sizeof(ledPins) / sizeof(ledPins[0]);
  
  // Set semua pin sebagai output
  for (int i = 0; i < ledCount; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  for (;;) {
    // LED berkedip bergantian: 4 → 5 → 6
    for (int i = 0; i < ledCount; i++) {
      Serial.println("Core " + String(xPortGetCoreID()) + ": LED PIN " + String(ledPins[i]) + " ON");
      digitalWrite(ledPins[i], HIGH);
      vTaskDelay(300 / portTICK_PERIOD_MS);  // Delay 300ms untuk setiap LED menyala
      
      Serial.println("Core " + String(xPortGetCoreID()) + ": LED PIN " + String(ledPins[i]) + " OFF");
      digitalWrite(ledPins[i], LOW);
      vTaskDelay(100 / portTICK_PERIOD_MS);  // Delay pendek sebelum LED berikutnya
    }
    
    // Delay tambahan sebelum mengulang sequence
    vTaskDelay(200 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  vTaskDelay(1000 / portTICK_PERIOD_MS);

  // Task 1: Group 1 LED (PIN 1,2,42) berkedip bersama di Core 0
  xTaskCreatePinnedToCore(
      ledGroup1Task,    // Fungsi task
      "LEDGroup1Task",  // Nama task
      2048,             // Ukuran stack
      NULL,             // Parameter
      1,                // Prioritas
      NULL,             // Handle task
      CORE_0            // Core ID (0)
  );

  // Task 2: Group 2 LED (PIN 4,5,6) berkedip bergantian di Core 1
  xTaskCreatePinnedToCore(
      ledGroup2Task, // Fungsi task
      "LEDGroup2Task",      // Nama task
      2048,                    // Ukuran stack
      NULL,                    // Parameter
      1,                       // Prioritas
      NULL,                    // Handle task
      CORE_1                   // Core ID (1)
  );
}

void loop() {
  // Loop utama tidak melakukan apa-apa karena semua pekerjaan dilakukan oleh tasks
  vTaskDelay(1000 / portTICK_PERIOD_MS);
}