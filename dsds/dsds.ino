#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  delay(100);

  Serial.println("RFID diagnostic");

  byte version = rfid.PCD_ReadRegister(
    rfid.VersionReg
  );

  Serial.print("Version register: 0x");
  Serial.println(version, HEX);

  Serial.print("Reader status: ");
  rfid.PCD_DumpVersionToSerial();

  Serial.println("Now scan your card.");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) {

    Serial.println("Card detected!");

    if (rfid.PICC_ReadCardSerial()) {
      Serial.println("UID successfully read!");

      Serial.print("UID: ");

      for (byte i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i], HEX);
        Serial.print(" ");
      }

      Serial.println();

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();

    } else {
      Serial.println("FAILED TO READ UID!");
    }

    delay(1000);
  }
}
