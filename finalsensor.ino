#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "A53s"
#define WIFI_PASSWORD "11111111"
#define API_KEY "AIzaSyBTLo5aqO8jXxi_kxnaxsw261WTzqD8Ny8"
#define DATABASE_URL "https://kelompok5-39a6c-default-rtdb.asia-southeast1.firebasedatabase.app/" 

#define TRIG_PIN D4
#define ECHO_PIN D3
#define MAX_DISTANCE 250
#define FLAME_PIN D5
#define BUZZER_PIN D8

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, 16, 2); 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

void setup() {
  Serial.begin(9600);
  pinMode(FLAME_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

   // Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase sign up successful");
  } else {
    Serial.printf("Firebase sign up failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Display initial message on LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
}

void loop() {
  // Ultrasonic sensor measurement
  long duration;
  float distance; // Mengubah tipe data menjadi float
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration * 0.034 / 2);

  // Flame sensor reading
  int flame = digitalRead(FLAME_PIN);

  // Print distance and flame status to Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  Serial.print("Flame status: ");
  Serial.println(flame == HIGH ? "Flame detected!" : "No flame detected");


  // Clear previous LCD content
  lcd.clear();
  lcd.setCursor(0, 0);

  // Display flame status on LCD
  if (flame == HIGH) {
    Serial.println("MENYALA ABANGKU");
    lcd.setCursor(2, 1);
    lcd.print("BAHAYA ADA API!!!");
    delay(1000);
    lcd.clear();
  } else {
    Serial.println("SITUASI AMAN");
    lcd.setCursor(2, 0);
    lcd.print("SITUASI AMAN ");
    delay(1000);
    lcd.clear();
  }

  if (flame == HIGH && distance <= 50) {
    Serial.println("BAHAYA Jangan mendekat!!!");
    Serial.print(distance);
    Serial.println(" cm JARAK BERBAHAYA");
    lcd.println("BAHAYA! ");
    lcd.setCursor(0, 1);
    lcd.print(distance);
    lcd.println(" cm JANGAN MENDEKAT");
    lcd.setCursor(0, 2);
    delay(1000);
    lcd.clear();
    tone(BUZZER_PIN, 500);
  } else {
    noTone(BUZZER_PIN);
  }

  if (Firebase.ready()) {
    // Flame status
    if (Firebase.RTDB.setInt(&fbdo, "sensor/flame_status", flame)) {
      Serial.println("Flame status data sent to Firebase successfully!");
    } else {
      Serial.printf("Failed to send flame status data to Firebase: %s\n", fbdo.errorReason().c_str());
    }

    // Distance
    if (Firebase.RTDB.setInt(&fbdo, "sensor/distance", distance)) {
      Serial.println("Distance data sent to Firebase successfully!");
    } else {
      Serial.printf("Failed to send distance data to Firebase: %s\n", fbdo.errorReason().c_str());
    }
  }

  // Delay for stability
  delay(1000); // 1 second delay
}