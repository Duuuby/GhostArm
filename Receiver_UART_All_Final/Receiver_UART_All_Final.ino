#include <HardwareSerial.h>
#include <Grove_LED_Bar.h>

HardwareSerial mySerial(1);

// LED bar
Grove_LED_Bar ledBar(6, 5, 1);

// Pin definitions
const int uartRxPin = 10;
const int testButtonPin = 1;
const int emergencyButtonPin = 3;
const int transistorPin = 4;

// UART level
int uartLevel = 0;

// Debounce timing
const unsigned long debounceDelay = 10;

// Test button state
bool testButtonState = false;
bool lastTestReading = false;
unsigned long lastTestDebounceTime = 0;

// Emergency button state
bool emergencyButtonState = false;
bool lastEmergencyReading = false;
unsigned long lastEmergencyDebounceTime = 0;

// System state
bool emergencyLocked = true;
bool waitForButtonRelease = false;

// Blink timing
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 150;
bool blinkState = false;

void setup() {
  Serial.begin(9600);

  mySerial.begin(9600, SERIAL_8N1, uartRxPin, -1);

  pinMode(testButtonPin, INPUT_PULLUP);
  pinMode(emergencyButtonPin, INPUT_PULLUP);
  pinMode(transistorPin, OUTPUT);

  digitalWrite(transistorPin, LOW);

  ledBar.begin();
  ledBar.setLevel(0);

  Serial.println("Receiver started");
}

void loop() {

  // Read UART data
  //if (mySerial.available()) {
    //uartLevel = mySerial.parseInt();

    //Serial.print("UART level: ");
    //Serial.println(uartLevel);
  //}
  if (mySerial.available() > 0) {
  uint8_t receivedByte = mySerial.read();

  if (receivedByte >= 0 && receivedByte <= 10) {
    uartLevel = receivedByte;
  }
}

  // Update button states
  updateButton(
    testButtonPin,
    testButtonState,
    lastTestReading,
    lastTestDebounceTime
  );

  updateButton(
    emergencyButtonPin,
    emergencyButtonState,
    lastEmergencyReading,
    lastEmergencyDebounceTime
  );

  // Wait until both buttons are released after reset
  if (waitForButtonRelease) {

    digitalWrite(transistorPin, LOW);

    if (!testButtonState && !emergencyButtonState) {
      waitForButtonRelease = false;
    }

    return;
  }

  // Enter emergency state
  if (emergencyButtonState && !emergencyLocked) {

    emergencyLocked = true;
    digitalWrite(transistorPin, LOW);

    Serial.println("Emergency active");
  }

  // Exit emergency state
  if (emergencyLocked &&
      testButtonState &&
      emergencyButtonState) {

    emergencyLocked = false;
    waitForButtonRelease = true;

    digitalWrite(transistorPin, LOW);
    ledBar.setLevel(uartLevel);

    Serial.println("Emergency reset");

    return;
  }

  // Emergency mode
  if (emergencyLocked) {

    digitalWrite(transistorPin, LOW);
    blinkLed10Fast();

    return;
  }

  // Update LED bar
  if (testButtonState) {
    ledBar.setLevel(10);
  } else {
    ledBar.setLevel(uartLevel);
  }

  // Update output
  if (uartLevel == 10 || testButtonState) {
    digitalWrite(transistorPin, HIGH);
  } else {
    digitalWrite(transistorPin, LOW);
  }
}

void updateButton(
  int pin,
  bool &buttonState,
  bool &lastReading,
  unsigned long &lastDebounceTime
) {

  bool reading = (digitalRead(pin) == LOW);

  if (reading != lastReading) {
    lastDebounceTime = millis();
    lastReading = reading;
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    buttonState = reading;
  }
}

void blinkLed10Fast() {

  if (millis() - lastBlinkTime >= blinkInterval) {

    lastBlinkTime = millis();
    blinkState = !blinkState;

    if (blinkState) {
      ledBar.setLevel(10);
    } else {
      ledBar.setLevel(0);
    }
  }
}