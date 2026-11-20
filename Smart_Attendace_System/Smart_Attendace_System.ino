#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 8
#define GREEN_LED 7
#define RED_LED 6

MFRC522 rfid(SS_PIN, RST_PIN);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2);

struct User {
  String uid;
  String name;
};

User users[] = {
  {"A1 B2 C3 D4", "Student 1"},
  {"E5 F6 G7 H8", "Student 2"}
};

const int totalUsers = sizeof(users) / sizeof(users[0]);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  Wire.begin();
  
  lcd.init();
  lcd.backlight();
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  if (!rtc.begin()) {
    lcd.print("RTC Error");
    while (1);
  }

  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  delay(1500);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();
  
  lcd.setCursor(0, 0);
  lcd.print(now.day() < 10 ? "0" : "");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month() < 10 ? "0" : "");
  lcd.print(now.month());
  lcd.print(" ");
  lcd.print(now.hour() < 10 ? "0" : "");
  lcd.print(now.hour());
  lcd.print(":");
  lcd.print(now.minute() < 10 ? "0" : "");
  lcd.print(now.minute());
  lcd.print(":");
  lcd.print(now.second() < 10 ? "0" : "");
  lcd.print(now.second());

  lcd.setCursor(0, 1);
  lcd.print("Scan your card..");

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String scannedUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      scannedUID += "0";
    }
    scannedUID += String(rfid.uid.uidByte[i], HEX);
    if (i < rfid.uid.size - 1) {
      scannedUID += " ";
    }
  }
  scannedUID.toUpperCase();

  bool authorized = false;
  String userName = "";

  for (int i = 0; i < totalUsers; i++) {
    if (scannedUID == users[i].uid) {
      authorized = true;
      userName = users[i].name;
      break;
    }
  }

  lcd.clear();
  if (authorized) {
    digitalWrite(GREEN_LED, HIGH);
    tone(BUZZER_PIN, 1000, 200);
    
    lcd.setCursor(0, 0);
    lcd.print("Welcome,");
    lcd.setCursor(0, 1);
    lcd.print(userName);

    Serial.print(userName);
    Serial.print(",");
    Serial.print(scannedUID);
    Serial.print(",");
    Serial.print(now.year());
    Serial.print("-");
    Serial.print(now.month());
    Serial.print("-");
    Serial.print(now.day());
    Serial.print(" ");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.println(now.second());

    delay(2000);
    digitalWrite(GREEN_LED, LOW);
  } else {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER_PIN, 400, 500);
    
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    lcd.setCursor(0, 1);
    lcd.print(scannedUID);

    Serial.print("UNKNOWN,");
    Serial.println(scannedUID);

    delay(2000);
    digitalWrite(RED_LED, LOW);
  }

  lcd.clear();
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
