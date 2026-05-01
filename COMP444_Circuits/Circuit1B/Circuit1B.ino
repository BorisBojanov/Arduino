int BAUDRATE =  9600;
int Pin      = 13;
int delayTime = 2000;
int potPosition;
char Analog = A0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);

  pinMode(Pin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  potPosition = analogRead(Analog);
  Serial.println(potPosition);

  digitalWrite(Pin,  HIGH);
  delay(potPosition);
  
  digitalWrite(Pin, LOW);
  delay(potPosition);
}
