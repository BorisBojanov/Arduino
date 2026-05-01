/*
Circuit 1C

Photo resistor analog input circuit. 
*/

int photoResist = 0; 
int threshold = 750;
int Pin = 13;
char analgPin = A0;
int Baud = 9600;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(Baud);
  pinMode(Pin, Baud);
}

void loop() {
  // put your main code here, to run repeatedly:
  photoResist = analogRead(analgPin);
  Serial.println(photoResist);

  //turn on the led if photo resitor is below threshold

  if(photoResist < threshold){
    digitalWrite(Pin, HIGH);
  } else {
    digitalWrite(PI, LOW);
  }
}
