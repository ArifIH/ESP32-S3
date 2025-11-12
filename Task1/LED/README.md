# Proyek Integrasi LED dengan ESP32

## Deskripsi Proyek
Proyek ini mengintegrasikan beberapa LED dengan menggunakan mikrokontroler ESP32 dan FreeRTOS untuk menjalankan dua task secara paralel di dua core yang berbeda. LED dikelompokkan menjadi dua grup yang masing-masing berkedip dengan pola yang berbeda. Task pertama mengendalikan LED pada Core 0, sementara task kedua mengendalikan LED pada Core 1. Ini merupakan demonstrasi penggunaan FreeRTOS untuk menjalankan berbagai task secara bersamaan.

## Konfigurasi Pin Hardware
Berikut adalah konfigurasi pin yang digunakan dalam proyek ini:

- **Pin LED**:
  - `LED1_PIN`: GPIO 40
  - `LED2_PIN`: GPIO 0
  - `LED3_PIN`: GPIO 19
  - `LED4_PIN`: GPIO 15
  - `LED5_PIN`: GPIO 54
  - `LED6_PIN`: GPIO 14

## Deskripsi Task
- **Task untuk Core 0: LED 1, 2, 3 berkedip bersama**  
  Task ini mengendalikan LED dengan pin `LED1_PIN`, `LED2_PIN`, dan `LED3_PIN` yang berkedip bersama-sama (nyala dan mati secara bersamaan) dengan interval 1 detik.

- **Task untuk Core 1: LED 4, 5, 6 berkedip bergantian**  
  Task ini mengendalikan LED dengan pin `LED4_PIN`, `LED5_PIN`, dan `LED6_PIN` yang berkedip secara bergantian (LED 4 menyala, kemudian LED 5, kemudian LED 6) dengan interval 300 ms untuk setiap LED menyala, dan 100 ms untuk menunggu sebelum LED berikutnya menyala.

## Instruksi Pengaturan

1. **Pengaturan Hardware**:
   - Sambungkan komponen LED sesuai dengan konfigurasi pin yang disebutkan di atas.
   - Pastikan semua LED terhubung ke pin GPIO yang sesuai dan Anda memiliki catu daya yang memadai untuk ESP32 dan LED.

2. **Instalasi Library**:
   - Di Arduino IDE, buka `Sketch` -> `Include Library` -> `Manage Libraries`.
   - Pastikan Anda telah menginstal pustaka berikut:
     - **Wire** (untuk komunikasi I2C jika diperlukan untuk perangkat lain)

3. **Mengunggah Kode**:
   - Pilih papan yang sesuai dari Arduino IDE (`ESP32 Dev Module`).
   - Unggah kode ke ESP32.

## Rangkuman Task dan Pembagian Core

| **Periferal**     | **Pin**      | **Task**               | **Deskripsi**                                             |
|-------------------|--------------|------------------------|-----------------------------------------------------------|
| **LED Group 1**   | GPIO 40, 0, 19 | `ledGroup1Task`         | LED berkedip bersama di Core 0.                           |
| **LED Group 2**   | GPIO 15, 54, 14 | `ledGroup2Task`         | LED berkedip bergantian di Core 1.                        |

## Pembagian Core
Proyek ini menggunakan **FreeRTOS** untuk membagi tugas ke dua core ESP32:

- **Core 0**:
  - `ledGroup1Task` (LED 1, 2, 3 berkedip bersama)

- **Core 1**:
  - `ledGroup2Task` (LED 4, 5, 6 berkedip bergantian)

## Vidio Output
- _Vidio&Documentation_: [GDRIVE](https://drive.google.com/drive/folders/1KZ1qvQGMUldeBAY5qFgIkaKy3y4ykiEV?usp=sharing)

