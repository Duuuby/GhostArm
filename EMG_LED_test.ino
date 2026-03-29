int sensorPin = A0;
int ledPin = 12;
double thresh = 90;  //threshold when LED lights up on certain muscle activity

void setup() {
  pinMode(ledPin,OUTPUT);
  Serial.begin(9600); //myabe increased baud rate (2e6) for faster data transmission
}

void loop() {
  float readValue = analogRead(sensorPin);  //reads the sensors current output value

  Serial.println(readValue);
  Serial.println(",0,600"); //Serial Plotter min and max borders to stop plotter from adjusting y-axis to every input value

  if(readValue>=thresh){
    digitalWrite(ledPin, HIGH);
  }else{
    digitalWrite(ledPin, LOW);
  }
}
