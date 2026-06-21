#include <SoftwareSerial.h>

const int potiPin = A0;

// SoftwareSerial(RX, TX)
// TX = Pin 7 zum ESP32-C3 RX
SoftwareSerial mySerial(6, 7);

void setup() {
  Serial.begin(9600);      // Serial Monitor
  mySerial.begin(9600);    // UART zu ESP32

  Serial.println("Arduino Uno Sender gestartet");
}

void loop() {
  int rawValue = analogRead(potiPin);          // 0 - 1023
  uint8_t level = (rawValue * 10) / 1024 + 1;  // 1 - 10

  mySerial.write(level);   // sendet EIN Byte: 1 bis 10

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Level gesendet: ");
  Serial.println(level);

  delay(100);
}