int delayTime = 1000;
int Pin13 = 13;

void setup() {
  // put your setup code here, to run once:
  pinMode(Pin13, OUTPUT);

}


void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(Pin13, HIGH);
  delay(delayTime);
  digitalWrite(Pin13, LOW);
  delay(delayTime);
}
