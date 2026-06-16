const int sensorPin = 2;         // Sensor pin
const int buttonPinF = 1;         //Button F(unction) pin
const int buttonPinT = 3;         //Button T(est) pin
const int senderPin = 7;         // Sender pin which sends a boolean to the receiver esp32
double thresh = 2.0;             //threshold when LED lights up on certain muscle activity, upper hysteresis border
double hystWidth = 0.5;  //width of hysteresis
double avrg;
double avrgVolt;

double startTimer = 0;
double pressDuration = 0;
bool buttonPressed = false;

const int potiPin = 4;
double refVolt = 3.3;  //3.3 V reference voltage
double potiVal = 0;
double potiVolt = 0;

bool threshReached = false;


void setup() {
  pinMode(sensorPin, INPUT);
  pinMode(buttonPinT, INPUT_PULLUP);
  pinMode(buttonPinF, INPUT_PULLUP);
  pinMode(senderPin, OUTPUT);
  pinMode(potiPin, INPUT);

  Serial.begin(9600);
  delay(1000);  // brief stabilization
}

void loop() {
  double readValue = analogRead(sensorPin);  //reads the sensors current output value
  avrg = avrgValue(30000, readValue);
  avrgVolt = (avrg / 4095) * refVolt;


  // Serial.println(readValue);
  // Serial.println(avrg);
  // Serial.println(",0,3.3");
  // Serial.println(",0,4095"); //Serial Plotter min and max borders to stop plotter from adjusting y-axis to every input value


  // if(avrg_volt>=thresh){    //if-condition for hysteresis
  //   digitalWrite(senderPin, true);
  // }else if(avrg_volt<=thresh-hystWidth){
  //   digitalWrite(senderPin, false);
  // }

  int buttonStateT = digitalRead(buttonPinT);
  // Serial.println(buttonStateT);

  if (buttonStateT == LOW) {  //Test Button part
    digitalWrite(senderPin, true);
    delay(10);  //debounce
  } else {
    digitalWrite(senderPin, false);
    delay(10);
  }
  Serial.println(digitalRead(senderPin));


  if (poti(potiPin, refVolt) >= thresh || threshReached) { //Test poti part
    digitalWrite(senderPin, true);
    threshReached = true;
  }
  if(poti(potiPin, refVolt) <= thresh-hystWidth){
    digitalWrite(senderPin, false);
    threshReached = false;
  }

  Serial.println(poti(potiPin, refVolt));
  Serial.println(digitalRead(senderPin));

  if(timer(buttonPinF)>=3000 && digitalRead(buttonPinF) == HIGH){
    Serial.println("Thresh Editing Mode on.");
    pressDuration = 0;
    buttonPressed = false;
  }



  // Serial.println(digitalRead(senderPin));
}

double avrgValue(int n, double value) {

  double avrg;
  double sum;
  int i = 1;

  while (i <= n) {  //sum up value until threshold is reached, then divide by number of values
    sum += value;
    i++;
    if (i == n) {
      sum = sum - value;
      avrg = sum / n;
      sum = 0;
    }
  }
  return avrg;
}

double timer(int pin) {
  int buttonState = digitalRead(pin);
  // Serial.println(buttonState);

  if (buttonState == LOW && !buttonPressed) {  // Button pressed
    startTimer = millis();
    buttonPressed = true;
    delay(50);  //debounce
  }

  if (buttonState == HIGH && buttonPressed) {  //Button was pressed
    pressDuration = millis() - startTimer;
  }

  if (buttonState == LOW && buttonPressed) {  //If button was released, save last millis() measurement
    pressDuration = millis() - startTimer;
    Serial.println(pressDuration);
  }
  return pressDuration;
}

double poti(int poti_pin, double reference_voltage) {
  potiVal = analogRead(poti_pin);
  potiVolt = (potiVal / 4095) * reference_voltage;
  // Serial.println(",0,3.3");
  // Serial.println(potiVal);
  // Serial.println(potiVolt);
  delay(10);

  return potiVolt;
}