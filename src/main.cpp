#include <MAX6675.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>

// Pin MAX6675
#define MAXSO  PB5
#define MAXSCK PB3
#define MAXCS  PB4

// Pin input
#define pbStart     PA1
#define pbStop      PA2
#define tombolMenu  PB12
#define tombolPlus  PB13
#define tombolMin   PB14
#define tombolSet   PB15

// Pin output
#define Pemantik PB11
#define Gas      PB10

MAX6675 thermocouple(MAXSCK, MAXCS, MAXSO);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pengaturan default
float targetTemp = 200.0;
float minTemp = 190.0;
unsigned long kontrolDurasi = 600000; // 10 menit

// EEPROM Address
#define ADDR_TARGET_TEMP    0
#define ADDR_MIN_TEMP       4
#define ADDR_DURATION       8

// Waktu
unsigned long pemantikStartMillis = 0;
unsigned long gasStartDelayMillis = 0;
unsigned long kontrolStartMillis = 0;
unsigned long lastSensorMillis = 0;
unsigned long lastCountdownMillis = 0;
unsigned long displayMessageUntil = 0; 
const unsigned long sensorInterval = 200;

bool sistemAktif = false;
bool kontrolDimulai = false;
bool pemantikAktif = false;
bool gasAktif = false;
bool pemantikDanGasNyalaSaatDingin = false;
bool sistemBaruMulai = false; // Flag baru untuk menangani start-up awal

float suhuTerakhir = NAN;

// Menu
enum MenuMode { MODE_NONE, MODE_TARGET, MODE_MINIMUM, MODE_DURASI };
MenuMode currentMenu = MODE_NONE;

// Debounce & Hold
unsigned long lastDebounce[6] = {0};
const unsigned long debounceDelay = 150;
const unsigned long holdDuration = 20;
unsigned long buttonPressTime[6] = {0};
bool buttonPrevState[6] = {false};

bool debounce(uint8_t pin, int index) {
  if (digitalRead(pin) == LOW && millis() - lastDebounce[index] > debounceDelay) {
    lastDebounce[index] = millis();
    return true;
  }
  return false;
}

bool detectHold(uint8_t pin, int index) {
  bool state = digitalRead(pin) == LOW;
  if (state && !buttonPrevState[index]) {
    buttonPressTime[index] = millis();
  } else if (!state && buttonPrevState[index]) {
    if (millis() - buttonPressTime[index] >= holdDuration) {
      buttonPrevState[index] = state; // Update state
      return true; // Dideteksi sebagai hold
    }
  }
  buttonPrevState[index] = state; // Update state sebelumnya
  return false; // Bukan hold atau belum dilepas
}

void mulaiPemantik() {
  digitalWrite(Pemantik, HIGH);
  pemantikStartMillis = millis();
  pemantikAktif = true;
  gasAktif = false; // Pastikan gas mati saat pemantik baru mulai (jika sebelumnya nyala)
  Serial.println("Pemantik ON");
}

void mulaiGas() {
  digitalWrite(Gas, HIGH);
  gasAktif = true;
  gasStartDelayMillis = millis();
  Serial.println("Gas ON");
}

void matikanSemua() {
  digitalWrite(Pemantik, LOW);
  digitalWrite(Gas, LOW);
  pemantikAktif = false;
  gasAktif = false;
  pemantikDanGasNyalaSaatDingin = false;
  Serial.println("Semua OFF");
}

void tampilMenu() {
  lcd.clear();
  switch (currentMenu) {
    case MODE_TARGET:
      lcd.print("Target Temp:");
      lcd.setCursor(0, 1);
      lcd.print("MaxTemp:"); lcd.print(targetTemp); lcd.print((char)223); lcd.print("C");
      break;
    case MODE_MINIMUM:
      lcd.print("Min Temp:");
      lcd.setCursor(0, 1);
      lcd.print("MinTemp:"); lcd.print(minTemp); lcd.print((char)223); lcd.print("C");
      break;
    case MODE_DURASI:
      lcd.print("Durasi:");
      lcd.setCursor(0, 1);
      lcd.print("timer:"); lcd.print(kontrolDurasi / 60000); lcd.print(" menit");
      break;
    default:
      // Jika MODE_NONE, jangan tampilkan apa-apa di menu
      break;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(pbStart, INPUT_PULLUP);
  pinMode(pbStop, INPUT_PULLUP);
  pinMode(tombolMenu, INPUT_PULLUP);
  pinMode(tombolPlus, INPUT_PULLUP);
  pinMode(tombolMin, INPUT_PULLUP);
  pinMode(tombolSet, INPUT_PULLUP);

  pinMode(Pemantik, OUTPUT);
  pinMode(Gas, OUTPUT);
  matikanSemua(); // Pastikan semua output mati saat start-up

  lcd.init();
  lcd.backlight();

  // Membaca nilai dari EEPROM
  EEPROM.get(ADDR_TARGET_TEMP, targetTemp);
  EEPROM.get(ADDR_MIN_TEMP, minTemp);
  EEPROM.get(ADDR_DURATION, kontrolDurasi);

  // Validasi nilai yang dibaca dari EEPROM
  if (isnan(targetTemp) || targetTemp < 0 || targetTemp > 300) targetTemp = 100.0;
  if (isnan(minTemp) || minTemp < 0 || minTemp >= targetTemp) minTemp = 90.0; // minTemp harus lebih kecil dari targetTemp
  if (kontrolDurasi < 60000 || kontrolDurasi > 3600000) kontrolDurasi = 600000; // Min 1 menit, Max 60 menit

  lcd.setCursor(0, 0);
  lcd.print("GAS BURNER");
  delay(1000); 
  float t = thermocouple.readCelsius();
  lcd.setCursor(0, 1);
  if (isnan(t)) {
    lcd.print("Sensor ERROR");
    Serial.println("Sensor ERROR");
    while (1); // berhenti disini jika sensor error
  }
  lcd.print("Sensor OK");
  Serial.println("Sensor OK");
  delay(1000); 
  lcd.clear();
}

void tampilCountdown(unsigned long remainingMs) {
  lcd.setCursor(0, 1);
  unsigned long sisaDetik = remainingMs / 1000;
  unsigned long menit = sisaDetik / 60;
  unsigned long detik = sisaDetik % 60;
  lcd.print("Sisa: ");
  if (menit < 10) lcd.print("0");
  lcd.print(menit); lcd.print(":");
  if (detik < 10) lcd.print("0");
  lcd.print(detik); lcd.print(" ");

  Serial.print("Sisa: ");
  if (menit < 10) Serial.print("0");
  Serial.print(menit); Serial.print(":");
  if (detik < 10) Serial.print("0");
  Serial.print(detik); Serial.println(" ");
}

void loop() {
  unsigned long currentMillis = millis();

  //tombol Start
  if (detectHold(pbStart, 0) && !sistemAktif) {
    sistemAktif = true;
    sistemBaruMulai = true; // Set flag: Sistem baru saja dimulai
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistem Mulai");
    mulaiPemantik(); // Pemantik dinyalakan untuk pertama kali
    displayMessageUntil = currentMillis + 500; // Tampilkan pesan "Sistem mulai" selama 0.5 detik
    Serial.println("Sistem AKTIF: Memulai pemanasan awal.");
  }

  // tombol Stop
  if (detectHold(pbStop, 1) && sistemAktif) {
    sistemAktif = false;
    kontrolDimulai = false;
    matikanSemua();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sistem STOP");
    displayMessageUntil = currentMillis + 1000; // Tampilkan pesan "Sistem STOP" selama 1 detik
    Serial.println("Sistem STOPPED");
    currentMenu = MODE_NONE;  // Saat sistem berhenti, pastikan keluar dari mode menu
  }

  // Pembacaan sensor suhu
  if (currentMillis - lastSensorMillis >= sensorInterval) {
    lastSensorMillis = currentMillis;
    float suhu = thermocouple.readCelsius();
    //suhu = suhu - 2.0; 

    if (!isnan(suhu)) {
      // Hanya update LCD dan Serial jika suhu berubah secara signifikan
      if (isnan(suhuTerakhir) || abs(suhu - suhuTerakhir) >= 0.01) {
        // Tampilkan suhu hanya jika tidak dalam mode menu dan tidak ada pesan temporer
        if (currentMenu == MODE_NONE && currentMillis > displayMessageUntil) {
          lcd.setCursor(0, 0);
          lcd.print("Temp: ");
          lcd.print(suhu, 2);
          lcd.print((char)223); lcd.print("C   ");
        }
        Serial.print("Temp: ");
        Serial.print(suhu, 2);
        Serial.print((char)223); Serial.println("C   ");
        suhuTerakhir = suhu;
      }

      if (sistemAktif) {
        // Logika kontrol dimulai saat suhu mencapai targetTemp
        if (!kontrolDimulai && suhu >= targetTemp) {
          kontrolStartMillis = currentMillis;
          kontrolDimulai = true;
          Serial.println("Timer Kontrol Dimulai!");
          sistemBaruMulai = false; // Reset flag sistemBaruMulai karena target sudah tercapai
        }

        // Penanganan countdown jika kontrol sudah dimulai
        if (kontrolDimulai) {
          unsigned long elapsed = currentMillis - kontrolStartMillis;
          if (elapsed >= kontrolDurasi) {
            // Kontrol Selesai
            sistemAktif = false;
            kontrolDimulai = false;
            matikanSemua();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Kontrol Selesai");
            displayMessageUntil = currentMillis + 1500; // Tampilkan pesan selama 1.5 detik
            Serial.println("Kontrol Selesai!");
            currentMenu = MODE_NONE; // Pastikan keluar dari mode menu saat selesai
          } else if (currentMillis - lastCountdownMillis >= 1000) {
            // Update countdown setiap detik
            lastCountdownMillis = currentMillis;
            // Tampilkan countdown hanya jika tidak dalam mode menu dan tidak ada pesan temporer
            if (currentMenu == MODE_NONE && currentMillis > displayMessageUntil) {
              tampilCountdown(kontrolDurasi - elapsed);
            }
          }
        }

        // Kontrol Pemantik dan Gas berdasarkan Suhu
        // Jika suhu sudah mencapai target, matikan semua
        if (suhu >= targetTemp) {
          matikanSemua();
          sistemBaruMulai = false; // Jika sistem baru mulai, matikan flag sistemBaruMulai
        }
        // Jika suhu di bawah minTemp DAN pemantik tidak aktif DAN TIDAK DALAM FASE START-UP AWAL
        // Maka nyalakan pemantik
        else if (suhu < minTemp && !pemantikAktif && !pemantikDanGasNyalaSaatDingin && !sistemBaruMulai) {
          mulaiPemantik();
          pemantikDanGasNyalaSaatDingin = true; // Set flag ini agar tidak terus-menerus memicu
          Serial.println("Suhu di bawah minTemp, menyalakan pemantik.");
        }

        // Reset pemantikDanGasNyalaSaatDingin jika suhu sudah naik kembali di atas minTemp
        if (suhu >= minTemp) {
          pemantikDanGasNyalaSaatDingin = false;
          // Juga reset flag sistemBaruMulai jika suhu sudah mencapai minTemp
          sistemBaruMulai = false;
        }
      }
    } else {
      // Penanganan Sensor Error
      if (currentMenu == MODE_NONE && currentMillis > displayMessageUntil) {
        lcd.setCursor(0, 0);
        lcd.print("Sensor ERROR    ");
      }
      Serial.println("Sensor ERROR!");
    }
  }

  // Logika Pemantik ON/OFF (untuk durasi pemantik saja)
  if (pemantikAktif) {
    if (!gasAktif && millis() - pemantikStartMillis >= 3000) { // Gas menyala 3 detik setelah pemantik
      mulaiGas();
    }
    if (millis() - pemantikStartMillis >= 10000) { // Pemantik mati setelah 10 detik
      digitalWrite(Pemantik, LOW);
      pemantikAktif = false;
      Serial.println("Pemantik OFF (timeout)");
    }
  }

  // Penanganan tombol Menu (Hanya bisa ditekan saat sistem tidak aktif)
  if (!sistemAktif && debounce(tombolMenu, 2)) {
    currentMenu = (MenuMode)((currentMenu + 1) % 4); // Berpindah antar MODE_NONE, MODE_TARGET, MODE_MINIMUM, MODE_DURASI
    if (currentMenu != MODE_NONE) {
      tampilMenu(); // Tampilkan menu jika bukan MODE_NONE
      displayMessageUntil = 0; // Reset waktu pesan agar menu tidak langsung tertimpa oleh pesan suhu/countdown
    } else {
      lcd.clear(); // Bersihkan LCD saat keluar dari menu
      Serial.println("Keluar dari Menu");
    }
  }

  // Penanganan tombol Plus, Min, Set hanya saat dalam mode menu DAN sistem tidak aktif
  if (currentMenu != MODE_NONE && !sistemAktif) {
    if (debounce(tombolPlus, 3)) {
      switch (currentMenu) {
        case MODE_TARGET: targetTemp += 1.0; break;
        case MODE_MINIMUM: minTemp += 1.0; break;
        case MODE_DURASI: kontrolDurasi += 60000; break; // Tambah 1 menit
        default: break;
      }
      tampilMenu(); // Update tampilan menu setelah perubahan
      Serial.print("Setting: "); Serial.print(currentMenu); Serial.print(", Value: ");
      if (currentMenu == MODE_TARGET) Serial.println(targetTemp);
      else if (currentMenu == MODE_MINIMUM) Serial.println(minTemp);
      else if (currentMenu == MODE_DURASI) Serial.println(kontrolDurasi);
    }

    if (debounce(tombolMin, 4)) {
      switch (currentMenu) {
        case MODE_TARGET: if (targetTemp > 0) targetTemp -= 1.0; break;
        case MODE_MINIMUM: if (minTemp > 0) minTemp -= 1.0; break;
        case MODE_DURASI: if (kontrolDurasi > 60000) kontrolDurasi -= 60000; break; // Kurang 1 menit, min 1 menit
        default: break;
      }
      tampilMenu(); // Update tampilan menu setelah perubahan
      Serial.print("Setting: "); Serial.print(currentMenu); Serial.print(", Value: ");
      if (currentMenu == MODE_TARGET) Serial.println(targetTemp);
      else if (currentMenu == MODE_MINIMUM) Serial.println(minTemp);
      else if (currentMenu == MODE_DURASI) Serial.println(kontrolDurasi);
    }

    if (debounce(tombolSet, 5)) {
      // Simpan nilai ke EEPROM
      EEPROM.put(ADDR_TARGET_TEMP, targetTemp);
      EEPROM.put(ADDR_MIN_TEMP, minTemp);
      EEPROM.put(ADDR_DURATION, kontrolDurasi);
      lcd.setCursor(0, 1);
      lcd.print("Disimpan!       "); // Pesan konfirmasi
      Serial.println("Pengaturan Disimpan!");
      displayMessageUntil = currentMillis + 1000; // Tampilkan pesan "Disimpan!" selama 1 detik
      currentMenu = MODE_NONE; // Keluar dari mode menu setelah disimpan
    }
  }

  // Logika untuk menghapus pesan sementara di LCD
  // Hanya bersihkan layar jika currentMenu bukan mode menu
  if (displayMessageUntil > 0 && currentMillis >= displayMessageUntil) {
    if (currentMenu == MODE_NONE) { // Hanya clear jika tidak sedang di menu pengaturan
      lcd.clear();
    }
    displayMessageUntil = 0; // Reset timer pesan
  }
}