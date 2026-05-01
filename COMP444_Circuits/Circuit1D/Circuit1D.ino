/*
Circuit 1C

Photo resistor analog input circuit. 
*/

int photoResist = 0;
int potResits   = 0;
int threshold = 900;
int redPin = 9;     //PWM Pin
int greenPin = 10;  //PWM Pin
int bluePin = 11;   //PWM Pin

char analgPin0 = A0;
char analgPin1 = A1;
int Baud = 9600;
int time = 200;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(Baud);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  photoResist = analogRead(analgPin0);
  potResits   = analogRead(analgPin1);

  Serial.print("photoresistor Vale: ");
  Serial.print(photoResist);
  Serial.print(" potentiometer value: ");
  Serial.println(potResits);

  //if its dark turn on
  if(photoResist < threshold){
  //nested ifs for different levels of darkness
    if (potResits > 0 && potResits <= 150) red();
    if (potResits > 150 && potResits <= 300) orange();
    if (potResits > 300 && potResits <= 450) yellow();
    if (potResits > 450 && potResits <= 600) green();
    if (potResits > 600 && potResits <= 750) cyan();
    if (potResits > 750 && potResits <= 900) blue();
    if (potResits > 900) magenta();

  } else {
    turnOff();
  }
  delay(time);
}


void red () {
  // values between 0-100
  analogWrite(redPin, 100);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}

void orange () {
  // values between 0-100
  analogWrite(redPin, 100);
  analogWrite(greenPin, 60);
  analogWrite(bluePin, 0);
}

void yellow () {
  // values between 0-100
  analogWrite(redPin, 100);
  analogWrite(greenPin, 100);
  analogWrite(bluePin, 0);
}

void green () {
  // values between 0-100
  analogWrite(redPin, 0);
  analogWrite(greenPin, 100);
  analogWrite(bluePin, 0);
}

void cyan () {
  // values between 0-100
  analogWrite(redPin, 0);
  analogWrite(greenPin, 100);
  analogWrite(bluePin, 100);
}

void blue () {
  // values between 0-100
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 100);
}

void magenta () {
  // values between 0-100
  analogWrite(redPin, 100);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 100);
}

void turnOff() {
    // values between 0-100
  analogWrite(redPin, 0);
  analogWrite(greenPin, 0);
  analogWrite(bluePin, 0);
}



