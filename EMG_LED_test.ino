
int sensorPin = A0;
int ledPin = 12;
double threshHigh = 90;  //threshold when LED lights up on certain muscle activity, upper hysteresis border
double threshLow = 10;  //lower hysteresis border
double avrg;

void setup() {
  pinMode(ledPin,OUTPUT);
  Serial.begin(2e6); //myabe increased baud rate (2e6) for faster data transmission
  
}

void loop() {
  double readValue = analogRead(sensorPin);  //reads the sensors current output value

  // Serial.print("readValue: ");
  // Serial.println(rueadValue);
  // Serial.print("Borders: ");
  Serial.println(",0,600"); //Serial Plotter min and max borders to stop plotter from adjusting y-axis to every input value

  if(readValue>=threshHigh){
    digitalWrite(ledPin, HIGH);
  }else if(readValue<=threshLow){
    digitalWrite(ledPin, LOW);
  }

  avrg = avrgValue(30000,readValue);
  // Serial.print("avrg: ");
  Serial.println(avrg);

  
}

double avrgValue(int n, double value){

  double avrg; 
  double sum;
  int i=1;

  while(i<=n){
    sum += value;
    i++;
    if(i==n){
      avrg = sum/n;
      sum=0;
    }
  }
  return avrg;
}