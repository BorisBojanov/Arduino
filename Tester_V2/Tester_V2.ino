#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//Goard Name: FireBeetle-ESP32: by esp32
#define OLED_WIDTH 128
#define OLED_HEIGHT 32
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET);
// char A0;
// char A1;
// char A2;
const bool WEAPON_CHECK_MODE = 1;
const bool BODY_WIRE_CHECK_MODE = 0; 
#define SENSOR_PIN_A0 A0 // Pin connected to the sensor, checkts the voltage for the equipment
#define SENSOR_PIN_A1 A1
#define SENSOR_PIN_A2 A2
#define DigitalInputPIN D7  // Analog input pin connected to D7
#define ALine D2 // Pin connected to A line in the tester is high or low
#define CLine D3 // Pin connected to C Line on hte tester is high or low
#define LED D4 // Pin connected to the LED
/*
V1 = 3v3
R1 = 10ohms
R2 = 10ohms
Vout = 1.65V
Max value allowed for R2 is 13 ohms for weapon (+3)
Max value allowed for R2 is 12 ohms for body wire (+2)
*/

int mode; // Default mode

void setup() {
  pinMode(DigitalInputPIN, INPUT);
  pinMode(SENSOR_PIN_A0, INPUT);
  pinMode(SENSOR_PIN_A1, INPUT);
  pinMode(SENSOR_PIN_A2, INPUT);
  pinMode(ALine, OUTPUT);
  pinMode(CLine, OUTPUT);
  pinMode(LED, OUTPUT);
  Serial.begin(57600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Check your display's I2C address
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  // Don't proceed, loop forever
  }
  delay(50);  // Pause for 2 seconds
  display.clearDisplay();   // Clear the buffer
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0,0);     // Start at top-left corner
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  int sensorValue1 = analogRead(SENSOR_PIN_A0);
  int sensorValue2 = analogRead(SENSOR_PIN_A1);
  int sensorValue3 = analogRead(SENSOR_PIN_A2);
  bool checkMode = digitalRead(DigitalInputPIN);
  // The ESP32 uses a 12-bit ADC, meaning the values range from 0-4095
  // With a 3.3V reference voltage, each step represents approximately 0.0008 volts (3.3V / 4095 = 0.0008V)
  // Therefore, to get the voltage of the pin we must multiply the sensor value by 0.0008
  double voltage1 = sensorValue1 * 0.0008;
  double voltage2 = sensorValue2 * 0.0008;
  double voltage3 = sensorValue3 * 0.0008;

  if(digitalRead(DigitalInputPIN) == 1){
    mode = WEAPON_CHECK_MODE;
    Serial.println("Mode: Weapon Check");
  }
  else if(digitalRead(DigitalInputPIN) == 0){
    mode = BODY_WIRE_CHECK_MODE;
    Serial.println("Mode: Body Wire Check");
  }
 Serial.println(checkMode);
  display.clearDisplay();
  // mode specific code here
  switch(mode) {
    case WEAPON_CHECK_MODE:
        display.setCursor(0, 0);
        display.print("Mode: Weapon Check");
        Serial.println(voltage1, 6);
        Serial.println(voltage2, 6);
        Serial.println(voltage3, 6);
        //Set the Aline and Cline to low
        digitalWrite(ALine, LOW);
        digitalWrite(CLine, LOW);
        // wire A check for a foil
        if(voltage1 - voltage2 < 0.5 && voltage1 - voltage2 > -0.5){
            //write to the OLED the resistance readings
            //make a simbol for the test passing
            display.setCursor(54,8);
            display.print("Pass");
        } else {
             //write to the OLED the voltage readings
            display.setCursor(54,8);
            display.print("Fail");
        }

        // wire C check for a epee
        if(voltage3 - voltage2 < 0.5 && voltage3 - voltage2 > -0.5){
            display.setCursor(54,24);
            display.print("Pass");
        } else {
            display.setCursor(54,24);
            display.print("Fail");
        }        
      break;

            // Code body wire Check 
    case BODY_WIRE_CHECK_MODE:
        display.setCursor(0, 0);
        display.print("Mode: Body Wire Check");
        Serial.println(voltage1, 6);
        Serial.println(voltage2, 6);
        Serial.println(voltage3, 6);
        //set both lines high
        digitalWrite(ALine, HIGH);
        digitalWrite(CLine, HIGH);
        // wire 1 LED check
        if(voltage1 < 1.86 && voltage1 > 0.5){
            //write to the OLED the resistance readings
            //digitalWrite(LED, HIGH);
            display.setCursor(54,8);
            display.print("Pass");
        } else {
            display.setCursor(54,8);
            display.print("Fail");
        }
        // wire 2 LED check
        if(voltage2 < 1.86 && voltage2 > 0.5){
            //cant be less than 0.5 meaning nothing is plugged in

            //write to the OLED the resistance readings
            //digitalWrite(LED, HIGH);
            display.setCursor(54,16);
            display.print("Pass");

        } else {
            display.setCursor(54,16);
            display.print("Fail");
        }
        // wire 3 LED check
        if(voltage3 < 1.86 && voltage3 > 0.5){
            //write to the OLED the resistance readings
            //digitalWrite(LED, HIGH);
            display.setCursor(54,24);
            display.print("Pass");

        } else {
            display.setCursor(54,24);
            display.print("Fail");
        }
      break;
    default:
      // Default behavior
      digitalRead(DigitalInputPIN);
      break;
  }

  
  // Display the voltage or resistance values
  display.setCursor(0,8);  // Start at top-left corner
  display.print("V1:");
  display.print(voltage1, 2);

  display.setCursor(0, 16);
  display.print("V2:");
  display.print(voltage2, 2);

  display.setCursor(0, 24);
  display.print("V3:");
  display.println(voltage3, 2);

  display.display();  // Show the display buffer on the screen

  delay(1000);  // Delay to limit the frequency of mode checking and serial output
}
