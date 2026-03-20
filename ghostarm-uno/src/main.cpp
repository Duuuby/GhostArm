#include <Arduino.h>

// Buttons
#define BTN1 A1
#define BTN2 A2
#define BTN3 A3

// LEDs
#define LED1 13
#define LED2 12
#define LED3 11
#define LED4 10

#define LED_ON LOW
#define LED_OFF HIGH

// put function declarations here:
void runSnakeBounceAnimation();
void turnAllLedsOff();


void setup() {
  // put your setup code here, to run once:
  // Buttons als Input mit Pullup
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);

  // LEDs als Output
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  turnAllLedsOff();
}

void loop() {

  if(digitalRead(BTN1) == LOW)
  {
    runSnakeBounceAnimation();
  }
  else
  {
    turnAllLedsOff();
  }


}

// put function definitions here:
void runSnakeBounceAnimation() {
  // +1+2
  digitalWrite(LED1, LED_ON);
  delay(100);
  digitalWrite(LED2, LED_ON);
  delay(100);
  //+3-1 
  digitalWrite(LED1, LED_OFF);
  digitalWrite(LED3, LED_ON);
  delay(100);
  // +4-2
  digitalWrite(LED2, LED_OFF);
  digitalWrite(LED4, LED_ON);
  delay(100);
  // -3 (keep 4 on for bounce)
  digitalWrite(LED3, LED_OFF);
  delay(50);

  // flipped version (bounce back)
  // +4+3
  digitalWrite(LED4, LED_ON);
  delay(100);
  digitalWrite(LED3, LED_ON);
  delay(100);
  // +2-4
  digitalWrite(LED4, LED_OFF);
  digitalWrite(LED2, LED_ON);
  delay(100);
  // +1-3
  digitalWrite(LED3, LED_OFF);
  digitalWrite(LED1, LED_ON);
  delay(100);
  // -2 (keep 1 on for next cycle)
  digitalWrite(LED2, LED_OFF);
  delay(50);
}

void turnAllLedsOff() {
  digitalWrite(LED1, LED_OFF);
  digitalWrite(LED2, LED_OFF);
  digitalWrite(LED3, LED_OFF);
  digitalWrite(LED4, LED_OFF);
}
