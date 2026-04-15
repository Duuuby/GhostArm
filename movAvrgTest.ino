int i;
double value[] = {204,206,210,300,400,230,304,106,207,209};
double sum;
double avrg;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); //myabe increased baud rate (2e6) for faster data transmission
}

void loop() {
sum += value[i];
Serial.println(sum);
i++;
delay(1000);
if(i>9){
  i=0;
  sum = 0;
}

}
