//--------------------------Declarations----------------------------//
#include <Grove_LED_Bar.h>

Grove_LED_Bar bar(6, 5, 1); //Declaring LED-bar (Clock pin, Data pin, Orientation) -> 1 means from green to red

const uint8_t sensorPin = 2;   // Sensor pin
const uint8_t buttonPinF = 1;  //Button F(unction) pin
const uint8_t buttonPinT = 3;  //Button T(est) pin
const uint8_t senderPin = 7;   // Sender pin which sends a boolean to the receiver esp32

uint16_t refMaxBit = 4095;  //Max bit of 12-bit ADC of ESP32

float thresh = 2.0f;         //threshold when LED lights up on certain muscle activity, upper hysteresis border
float hystWidth = 0.5f;      //width of hysteresis
bool threshReached = false;  //for checking if thresh was reached

float avrg;
uint8_t sampleSize = 500;  //Sample size for moving average filter

unsigned long startTimer = 0;
unsigned long pressDuration = 0;
bool buttonPressed = false;
uint8_t buttonCounter = 1;  //Press counter for mode selection
bool lastButtonState = false;

const uint8_t potiPin = 4;
const float refVolt = 3.3f;  //3.3 V reference voltage

unsigned long lastPrintTime = 0;  //for checking how long ago the last print occured

float avrgValue(int n, uint8_t pin);  //Pre-declarations of functions
float timer(uint8_t pin);
float poti(uint8_t poti_pin);
void ledScale(uint8_t pin, int refMaxBit);
float threshEditing(uint8_t potiPin, uint8_t buttonPin);

//--------------------------Setup----------------------------//
void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buttonPinT, INPUT_PULLUP);
  pinMode(buttonPinF, INPUT_PULLUP);
  pinMode(senderPin, OUTPUT);
  pinMode(potiPin, INPUT);

  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, -1, 7);

  bar.begin();
  delay(1000);  // brief stabilization
}

//--------------------------Loop----------------------------//
void loop() {
  float currentHoldTime = timer(buttonPinF);  //Start timer on press
  int currentButtonState = digitalRead(buttonPinF); //check button state

  //Mode Selection (Switching/Editing)
  if (currentHoldTime >= 3000 && currentButtonState == LOW) {
    thresh = threshEditing(potiPin, buttonPinF);  //Editing
  }

  if (lastButtonState == LOW && currentButtonState == HIGH) {
    //Switching modes (1->2->3->1->...)
    unsigned long duration = millis() - startTimer;
    if (duration < 3000 && duration > 50) { //Check for short press (less than 3 sec, more than 50 ms)
      buttonCounter++;
      if (buttonCounter > 3) buttonCounter = 1; //Button counter for defining modes
      if (buttonCounter == 1) {
        for (int i = 0; i < 3; i++) {  //Start-animation (blinks 3 times)
          bar.setBits(0x000);          //Turn off section
          delay(300);
          bar.setBits(0x007);  //Bottom section (LED 1,2,3)
          delay(300);          //Show section for 300 ms
        }
      } else if (buttonCounter == 2) {
        for (int i = 0; i < 3; i++) {
          bar.setBits(0x000);  //Turn off section
          delay(300);
          bar.setBits(0x078);  //Middle section (LED 4,5,6,7)
          delay(300);
        }
      } else if (buttonCounter == 3) {
        for (int i = 0; i < 3; i++) {
          bar.setBits(0x000);
          delay(300);
          bar.setBits(0x380);  //Top section (LED 8,9,10)
          delay(300);
        }
      }
    }
  }

  //EMG-Input
  if (buttonCounter == 1) {
    avrg = avrgValue(sampleSize, sensorPin);  //Computes average bits of sensor output
    float avrgVolt = avrg * (refVolt / (float)refMaxBit); //calculates the according voltage from previous computation

    ledScale(sensorPin, refMaxBit); //Scales LED-bar to certain voltage width (0-3.3 V)

    //Sending avrg value bits vie uart
    uint16_t outVal = (uint16_t)avrg; //Defining avrg as uint16_t (16 Bits)
    Serial1.write((outVal >> 8) & 0xFF);  //High-byte, bits are moved 8 bits to the right, low-byte values are deleted, 0 are added to the left
    Serial1.write(outVal & 0xFF);         //Low-byte values are added with the second 8-bit-package

    if (millis() - lastPrintTime > 200) { //Output control, prevent Serial.monitor spaming
      Serial.print("|--Sensor Mode--| Average Voltage: ");
      Serial.print(avrgVolt);
      Serial.print(" V |--Output UART--| Bits: ");
      Serial.println(outVal);
      lastPrintTime = millis();
    }
  }


  //Test-Button
  if (buttonCounter == 2) {
    int buttonStateT = digitalRead(buttonPinT); 
    uint16_t outTest = (buttonStateT == LOW) ? 4095 : 0;  //Convert buttonStateT from bool to uint16_t, (condition)? if true: if false
    Serial1.write((outTest >> 8) & 0xFF);  //High-byte
    Serial1.write(outTest & 0xFF);         //Low-byte

    if (buttonStateT == LOW) {  //Test Button part animation if false/true
      bar.setBits(0x3ff);
    } else {
      bar.setBits(0x0);
    }
    delay(30);  //debounce

    if (millis() - lastPrintTime > 200) { //Spam-controll
      Serial.print("|--Test Button Mode--| State: ");
      Serial.print(digitalRead(buttonPinT));
      Serial.print(" |--Output UART--| Bits: ");
      Serial.println(outTest);
      lastPrintTime = millis();
    }
  }

  //Potentiometer
  if (buttonCounter == 3) {
    ledScale(potiPin, refMaxBit);
    float potiVal = poti(potiPin);  //Read poti
    float potiVolt = potiVal * (refVolt / (float)refMaxBit);  //convert poti bits to voltage values

    uint16_t outPoti = (uint16_t)potiVal;
    Serial1.write((outPoti >> 8) & 0xFF);  //High-byte
    Serial1.write(outPoti & 0xFF);         //Low-byte
    if (millis() - lastPrintTime > 200) {
      Serial.print("|--Potentiometer Mode--| Voltage: ");
      Serial.print(potiVolt);
        Serial.print(" V |--Output UART--| Bits: ");
      Serial.println(outPoti);
      lastPrintTime = millis();
    }
  }
  lastButtonState = currentButtonState; //update buttonPush for difference between holding an mode selection
}  //Void loop end

//--------------------------Functions----------------------------//
//Avrg EMG-value
float avrgValue(int n, uint8_t pin) {
  uint32_t sum = 0;

  for (int i = 0; i < n; i++) {
    sum += analogRead(pin);     //Read pin value and add to sum-variable
    if (i % 50 == 0) delay(1);  //Yield back to ESP32 to prevent crash of n is very large (Taking a break every 50 values to give CPU time to do computations.)
  }

  return (float)sum / n;  //Return float value of "sum of values"/"number of values"
}

//Timer
float timer(uint8_t pin) {
  int buttonState = digitalRead(pin);  //Read button state

  //First time pressing the button, staring timer
  if (buttonState == LOW && !buttonPressed) {  //If button state is low and  button is being pressed start timer. After first press, !buttonPressed will be false --> skips condition
    startTimer = millis();
    buttonPressed = true;  //Sets variable on true bcs button was pressed
    delay(30);             //debounce
  }
  //Holding the button, counting timer
  if (buttonState == LOW && buttonPressed) {  //If button state is low and button is being pressed (true), calculate time that passed since first press-detection.
    pressDuration = millis() - startTimer;
    return (float)pressDuration;  //Returns current time
  }
  //Releasing the button, stopping timer
  if (buttonState == HIGH && buttonPressed) {  //If button isn´t being pressed anymore and button was pressed (true), reset variables to starting values.
    pressDuration = 0;                         //Set pressDuration and buttonPressed to beginning values
    buttonPressed = false;
  }
  return 0;
}

//Poti-value
float poti(uint8_t potiPin) {
  return (float)analogRead(potiPin);  //Read poti bits
}

//LedScaling to Bits
void ledScale(uint8_t pin, int refMaxBit) {  //Scaling bits of a certain pin (ADC) to the 10 LED-levels of the LED-bar
  int val = analogRead(pin);
  int ledLevel = map(val, 0, refMaxBit, 0, 10);
  bar.setLevel(ledLevel);
}

//Threshold Editing-mode
float threshEditing(uint8_t potiPin, uint8_t buttonPin) {  //Still interfering with other outputs somehow
  float newThresh = 0;
  bool editing = true;
  unsigned long lastEditPrint = 0;

  Serial.println("|-----------Thresh Editing Mode on-----------|");
  for (int i = 0; i < 3; i++) {  //Start-animation
    bar.setBits(0x0);
    delay(300);
    bar.setBits(0x3ff);
    delay(300);
  }
  while (editing) {
    float currentHoldTime = timer(buttonPin);
    float currentVolt = analogRead(potiPin) * (refVolt / refMaxBit);
    if (currentHoldTime >= 3000 && digitalRead(buttonPin) == LOW) {  //if function button is pressed longer than 3 sec and is being pressed, end while-loop
      newThresh = currentVolt;
      Serial.println("New threshold saved.");
      editing = false;  
    } else {  //Scaling LED bar to current poti values
      ledScale(potiPin, refMaxBit);
      if (millis() - lastEditPrint > 150) {
        Serial.print("Current Thresh: ");
        Serial.print(currentVolt);
        Serial.println(" V");
        lastEditPrint = millis();
      }
    }
    delay(1); //for computation
  }
  for (int i = 0; i < 3; i++) {  //End-animation
    bar.setBits(0x0);
    delay(300);
    ledScale(potiPin, refMaxBit);
    delay(300);
  }
  ledScale(potiPin, refMaxBit);  //1.2 sec freeze-effect, for visual harmony
  delay(1200);

  while (digitalRead(buttonPin) == LOW) {
    delay(30);  //debounce
  }
  return newThresh;
}
