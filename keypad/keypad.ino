
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

int r1 = 10;
int r2 = 9;
int r3 = 8;
int r4 = 7;

int c1 = 6;
int c2 = 5;
int c3 = 4;

LiquidCrystal_I2C lcd(0x27, 16, 2);

String keypressed = "";
String textEntered = "";

int cursorPosition = 0;
bool keyWasPressed = false;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(r1, OUTPUT);
  pinMode(r2, OUTPUT);
  pinMode(r3, OUTPUT);
  pinMode(r4, OUTPUT);

  pinMode(c1, INPUT_PULLUP);
  pinMode(c2, INPUT_PULLUP);
  pinMode(c3, INPUT_PULLUP);

  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);

  lcd.clear();
}

void loop() {
  keypressed = "";

  digitalWrite(r1, LOW);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);

  if (digitalRead(c1) == LOW) keypressed = "1";
  else if (digitalRead(c2) == LOW) keypressed = "2";
  else if (digitalRead(c3) == LOW) keypressed = "3";

  if (keypressed == "") {
    digitalWrite(r1, HIGH);
    digitalWrite(r2, LOW);

    if (digitalRead(c1) == LOW) keypressed = "4";
    else if (digitalRead(c2) == LOW) keypressed = "5";
    else if (digitalRead(c3) == LOW) keypressed = "6";
  }

  if (keypressed == "") {
    digitalWrite(r2, HIGH);
    digitalWrite(r3, LOW);

    if (digitalRead(c1) == LOW) keypressed = "7";
    else if (digitalRead(c2) == LOW) keypressed = "8";
    else if (digitalRead(c3) == LOW) keypressed = "9";
  }

  if (keypressed == "") {
    digitalWrite(r3, HIGH);
    digitalWrite(r4, LOW);

    if (digitalRead(c1) == LOW) keypressed = "*";
    else if (digitalRead(c2) == LOW) keypressed = "0";
    else if (digitalRead(c3) == LOW) keypressed = "#";
  }

  digitalWrite(r1, HIGH);
  digitalWrite(r2, HIGH);
  digitalWrite(r3, HIGH);
  digitalWrite(r4, HIGH);

  // Process only a new key press
  if (keypressed != "" && !keyWasPressed) {
    keyWasPressed = true;

    if (keypressed == "*") {
      // BACKSPACE
      if (textEntered.length() > 0) {
        textEntered.remove(textEntered.length() - 1);
        updateLCD();
      }
    }
    else if (keypressed == "#") {
      // CLEAR EVERYTHING
      textEntered = "";
      updateLCD();
    }
    else {
      // Add the new character
      if (textEntered.length() < 32) {
        textEntered += keypressed;
        updateLCD();
      }
    }
  }

  // Wait until the key is released
  if (keypressed == "") {
    keyWasPressed = false;
  }

  delay(30);
}

void updateLCD() {
  lcd.clear();

  // Print characters 0–15 on row 1
  lcd.setCursor(0, 0);
  for (int i = 0; i < textEntered.length() && i < 16; i++) {
    lcd.print(textEntered[i]);
  }

  // Print characters 16–31 on row 2
  if (textEntered.length() > 16) {
    lcd.setCursor(0, 1);
    for (int i = 16; i < textEntered.length(); i++) {
      lcd.print(textEntered[i]);
    }
  }
}
