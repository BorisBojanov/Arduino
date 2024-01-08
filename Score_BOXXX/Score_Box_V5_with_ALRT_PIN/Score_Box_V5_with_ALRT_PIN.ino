//Score_Box_V5_with_ALRT_PIN
#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ADS1X15.h>
#include <Arduino.h>

//ADC info
// The ADC input range (or gain) can be changed via the following
// functions, but be careful never to exceed VDD +0.3V max, or to
// exceed the upper and lower limits if you adjust the input range!
// Setting these values incorrectly may destroy your ADC!
//                                                                ADS1015  ADS1115
//                                                                -------  -------
// ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
// ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
// ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
// ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
// ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
// ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
Adafruit_ADS1115 ads_OnTarget;
Adafruit_ADS1115 ads_OffTarget;
Adafruit_ADS1115 ads_Short;
// ADS1115 Addresses
const short converReg = 0b00000000;
const short CfgReg    = 0b00000001;
const short LoThresh   = 0b00000010;
const short HiThresh   = 0b00000011;
// FastLED Setup
#define NUMPIXELS 64 // number of neopixels in strip
struct CRGB pixels[NUMPIXELS];
struct CRGB pixels2[NUMPIXELS];
// Enumeration for modes
typedef enum {
 FOIL,
 EPEE,
 SABRE
} WeaponMode;
WeaponMode currentMode;
//Global Variables
// Pin Setup
// Gpios 6, 7, 8, 9, 10 and 11, 12 are a no go for ESP32 to use
// Gpio 0 is always high at 4096
// Gpio 35 is always high at 4096
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceTime = 200; // Debounce time in milliseconds
const uint16_t BAUDRATE = 115200; // baudrate of the serial debug interface
const uint16_t LIGHTTIME = 3000; // length of time the lights are kept on after a hit (ms)
const uint16_t BUZZERTIME = 1000; //// length of time the buzzer is kept on after a hit (ms)
const uint16_t DEBOUNCE = 50; // debounce time for the buttons (ms)
const uint8_t ALRT_OnTarget_PIN = 34;
const uint8_t ALRT_OffTarget_PIN = 35;
const uint8_t ALRT_Short_PIN = 36;
const uint8_t modeChange_PIN = 25;
const uint8_t buzzer_PIN = 28; 
#define LED_PIN   19 //27, 26 gets mad at me for assigning those pins
#define LED_PIN_2 26
bool ALRT_OnTarget = false;
bool ALRT_OffTarget = false;
bool ALRT_Short = false;
bool modeChange =  false;
const uint8_t FOIL_MODE  = 0;
const uint8_t EPEE_MODE  = 1;
const uint8_t SABRE_MODE = 2;
uint8_t MODE_INT = FOIL_MODE;
//if variables are used by ISR mkae them Volatile
//if  variables change often and need to be loaded quickly make them Volatile
volatile bool lockedOut    = false;
volatile bool depressedA   = false;
volatile bool depressedB   = false;
volatile bool hitOnTargA  = false;
volatile bool hitOffTargA = false;
volatile bool hitOnTargB  = false;
volatile bool hitOffTargB = false;
volatile long depressAtime = 0;
volatile long depressBtime = 0;
volatile int weaponA = 0;
volatile int weaponB = 0;
volatile int lameA   = 0;
volatile int lameB   = 0;
volatile int groundA = 0;
volatile int groundB = 0;
uint8_t Lockout_Time_INT = 0;
uint8_t Depress_Time_INT = 1;
const long lockout [] = {300000,  45000, 120000};  // the lockout time between hits
const long depress [] = { 14000,   2000,   1000};  // the minimum amount of time the tip needs to be depressed

//small functions
void ALRT_OnTarget_ISR() {ALRT_OnTarget = true;}
void ALRT_OffTarget_ISR() {ALRT_OffTarget = true;}
void ALRT_Short_ISR() {ALRT_Short = true;}

//When mode button is pressed this function is excicuted.
//No need to worry about speed of this ISR because the User will need to wait for LED confirmation of change
void modeChangeISR() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceTime) {
    // Handle the button press
    modeChange = true; // this raises the flag for the main loop to change modes
    lastInterruptTime = interruptTime;
  }
}
// MODE CHANGER
void ChangeTheMODE(){
  if(digitalRead(modeChange_PIN) == LOW) {
    if (MODE_INT == SABRE_MODE) {
      MODE_INT = FOIL_MODE;
    } else {
      MODE_INT += 1;
    }
  }
  //set_mode_leds();
  Serial.print("Mode changed to:");
  Serial.println(MODE_INT);
  lastInterruptTime = 0;
  modeChange = false;
}
//logic for the three modes
void foil(){
  const int foilMode[] = {300000, 14000};
  //it will be okay to have both ADCs assigning to the same value because they are not going to trigger at the same time
  weaponA = ads_OnTarget.readADC_SingleEnded(0);
  lameA = ads_OnTarget.readADC_SingleEnded(1);
  weaponB = ads_OnTarget.readADC_SingleEnded(2);
  lameB = ads_OnTarget.readADC_SingleEnded(3);

  weaponA = ads_OffTarget.readADC_SingleEnded(0);
  lameA = ads_OffTarget.readADC_SingleEnded(1);
  weaponB = ads_OffTarget.readADC_SingleEnded(2);
  lameB = ads_OffTarget.readADC_SingleEnded(3);
  long now = micros();//The long now = micros() variable is set earlier in the main loop.
  //_____Check for lockout___
  if (((hitOnTargA || hitOffTargA) && (depressAtime + foilMode[Lockout_Time_INT] < now)) 
                                      ||
      ((hitOnTargB || hitOffTargB) && (depressBtime + foilMode[Lockout_Time_INT] < now))) {
      lockedOut = true;
  }
  // weapon A
  if (hitOnTargA == false && hitOffTargA == false) {  // ignore if A has already hit
    // off target
    if (32000 < weaponA && lameB < 100) {
      if (!depressedA) {
        depressAtime = micros();
        depressedA   = true;
      } else {
        if (depressAtime + foilMode[Depress_Time_INT] <= micros()) {
            hitOffTargA = true;
        }
      }
    } else {
      // on target
      if ((17000 < weaponA && weaponA < 18000) && (17000 < lameB && lameB < 18000)) {
        if (!depressedA) {
          depressAtime = micros();
          depressedA   = true;
      } else {
          if (depressAtime + foilMode[Depress_Time_INT] <= micros()) {
            hitOnTargA = true;
          }
        }
      } else {
        // reset these values if the depress time is short.
        depressAtime = 0;
        depressedA   = 0;
      }
    }
  }  
  // weapon B
  if (hitOnTargB == false && hitOffTargB == false) { // ignore if B has already hit
    // off target
    if (32000 < weaponB && lameA < 100) {
      if (!depressedB) {
        depressBtime = micros();
        depressedB   = true;
      } else {
        if (depressBtime + foilMode[Depress_Time_INT] <= micros()) {
          hitOffTargB = true;
      }
    }
    } else {
    // on target
      if ((17000 < weaponB && weaponB < 18000) && (17000 < lameA && lameA < 18000)) {
        if (!depressedB) {
          depressBtime = micros();
          depressedB   = true;
        } else {
          if (depressBtime + foilMode[Depress_Time_INT] <= micros()) {
            hitOnTargB = true;
          }
        }
      } else {
        // reset these values if the depress time is short.
        depressBtime = 0;
        depressedB   = false;
      }
    }
  }
  //---------------------------------------------------------
}
void epee(){ 
  const int epeeMode[] = {45000, 2000};
  Serial.println("Inside: Handle EPEE Function");
  // Implement the specific behavior for a hit in Epee mode
  // read analog pins
  weaponA = ads_OnTarget.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
  lameA = ads_OnTarget.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
  weaponB = ads_OnTarget.readADC_SingleEnded(2);   // Read from channel 0 of the second ADS1115
  lameB = ads_OnTarget.readADC_SingleEnded(3);   // Read from channel 1 of the second ADS1115
  long now = micros();
  //_____Check for lockout___
  if ((hitOnTargA && (depressAtime + epeeMode[Lockout_Time_INT] < now)) 
                                    ||
    (hitOnTargB && (depressBtime + epeeMode[Lockout_Time_INT] < now))) {
    lockedOut = true;
  }

  // weapon A
  //  no hit for A yet    && weapon depress    && opponent lame touched
  if (hitOnTargA == false) {
    if (400 < weaponA && weaponA < 600 && 400 < lameA && lameA < 600) {
        if (!depressedA) {
          depressAtime = micros();
          depressedA   = true;
        } else {
          if (depressAtime + epeeMode[Depress_Time_INT] <= micros()) {
              hitOnTargA = true;
          }
        }
    } else {
        // reset these values if the depress time is short.
        if (depressedA == true) {
          depressAtime = 0;
          depressedA   = 0;
        }
    }
  }

  // weapon B
  //  no hit for B yet    && weapon depress    && opponent lame touched
  if (hitOnTargB == false) {
    if (400 < weaponB && weaponB < 600 && 400 < lameB && lameB < 600) {
        if (!depressedB) {
          depressBtime = micros();
          depressedB   = true;
        } else {
          if (depressBtime + epeeMode[Depress_Time_INT] <= micros()) {
              hitOnTargB = true;
          }
        }
    } else {
        // reset these values if the depress time is short.
        if (depressedB == !true) {
          depressBtime = 0;
          depressedB   = 0;
        }
    }
  }
}
void sabre(){
  const int sabreMode[] = {170000, 1000};
  long now = micros();
}

// Function to handle the selected mode
void handleMode() {
 switch (currentMode) {
    case FOIL:
      MODE_INT = FOIL_MODE;
      foil();
      break;
    case EPEE:
      MODE_INT = EPEE_MODE;
      epee();
      break;
    case SABRE:
      MODE_INT = SABRE_MODE;
      sabre();
      break;
    default:
      // Error handling if an invalid mode is selected
      break;
 }
}

void resetValues(){
  delay(BUZZERTIME);             // wait before turning off the buzzer
  digitalWrite(buzzer_PIN,  LOW);
  delay(LIGHTTIME-BUZZERTIME);   // wait before turning off the lights
  //Removed this from all Fill color commands and placed here to see if this command is being excicuted
  fill_solid(pixels, NUMPIXELS, CRGB::Black);
  fill_solid(pixels2, NUMPIXELS, CRGB::Black);
  FastLED.show();
  lastInterruptTime = 0;
  depressAtime = 0;
  depressBtime = 0;
  lockedOut    = false;
  depressedA   = false;
  depressedB   = false;
  hitOnTargA  = false;
  hitOffTargA = false;
  hitOnTargB  = false;
  hitOffTargB = false;
  ALRT_OnTarget = false;
  ALRT_OffTarget = false;
  ALRT_Short = false;
  modeChange =  false;
  delay(5);
}

void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Score_Box_V5_with_ALRT_PIN");
  pinMode(modeChange_PIN, INPUT_PULLUP);
  ads_OnTarget.begin(0x48);
  ads_OffTarget.begin(0x49);
  ads_Short.begin(0x4A);
  while (!Serial); // Wait for the serial port to connect
  if (!ads_OnTarget.begin(0x48)){
    Serial.println("Failed to initialize ADS1115: ads_OnTarget at 0x48!");
  }
  if (!ads_OffTarget.begin(0x49)){
    Serial.println("Failed to initialize ADS1115: ads_OffTarget at 0x49!");
  }
  if (!ads_Short.begin(0x4A)){
    Serial.println("Failed to initialize ADS1115: ads_Short at 0x4A!");
  }
  ads_OnTarget.setGain(GAIN_ONE);
  ads_OffTarget.setGain(GAIN_ONE);
  ads_Short.setGain(GAIN_ONE);
  ads_OnTarget.startComparator_SingleEnded(17000,18000);
  ads_OnTarget.startComparator_SingleEnded(17000,18000);
  ads_OffTarget.startComparator_SingleEnded(20000,32000);
  ads_OffTarget.startComparator_SingleEnded(20000,32000);
  ads_Short.startComparator_SingleEnded(-10,10);
  ads_Short.startComparator_SingleEnded(-10,10);
  Serial.println("ADC Range: (GAIN_ONE);  // 1x gain   +/- 4.096V  1 bit = 0.125mV");
  //FAST LED SETUP
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(pixels, NUMPIXELS);
  FastLED.addLeds<WS2812B, LED_PIN_2, GRB>(pixels2, NUMPIXELS);
  // Set an initial mode, for example, FOIL
  currentMode = FOIL;
  // Set up the interrupt pins
  
  attachInterrupt(digitalPinToInterrupt(ALRT_OnTarget_PIN),ALRT_OnTarget_ISR , CHANGE);
  attachInterrupt(digitalPinToInterrupt(ALRT_OffTarget_PIN),ALRT_OffTarget_ISR , CHANGE);
  attachInterrupt(digitalPinToInterrupt(ALRT_Short_PIN),ALRT_Short_ISR , CHANGE);
  attachInterrupt(digitalPinToInterrupt(modeChange_PIN),modeChangeISR , CHANGE);
  resetValues();
}

void loop() {
  //This loop should call one function that is a pointer to the mode that has been selected
  //This function should be called in a switch statement?
  //A function that checks the if any interrupts have set a flag to Ture
  int16_t weaponA, lameA, groundA, weaponB, lameB, groundB;
  weaponA = ads_OnTarget.readADC_SingleEnded(0);
  lameA = ads_OnTarget.readADC_SingleEnded(1);
  weaponB = ads_OnTarget.readADC_SingleEnded(2);
  lameB = ads_OnTarget.readADC_SingleEnded(3);

  weaponA = ads_OffTarget.readADC_SingleEnded(0);
  lameA = ads_OffTarget.readADC_SingleEnded(1);
  weaponB = ads_OffTarget.readADC_SingleEnded(2);
  lameB = ads_OffTarget.readADC_SingleEnded(3);


  //==========TESTING============
  Serial.print("ads_OnTarget (0x48) Channel 0 - weaponA: "); Serial.println(weaponA);
  Serial.print("ads_OnTarget (0x48) Channel 1 - weaponB: "); Serial.println(weaponB);
  Serial.print("ads_OnTarget (0x48) Channel 2 - lameA: "); Serial.println(lameA);
  Serial.print("ads_OffTarget (0x49) Channel 3 - lameB: "); Serial.println(lameB);
  //=============================
  Serial.print("ads_OffTarget (0x49) Channel 4 - weaponA: "); Serial.println(weaponA);
  Serial.print("ads_OffTarget (0x49) Channel 5 - weaponB: "); Serial.println(weaponB);
  Serial.print("ads_OffTarget (0x49) Channel 6 - lameA: "); Serial.println(lameA);
  Serial.print("ads_OffTarget (0x49) Channel 7 - lameB: "); Serial.println(lameB);
  //==========TESTING============

  // Check if any interrupts have set a flag to True
  if (ALRT_OnTarget){
    long now = micros();
    //This bool is triggered by the ALRT_OnTarget_ISR with a window ALRT of 17000 - 18000
    //A function that will do a thing

    if (!lockedOut) {
    if (((hitOnTargA || hitOffTargA) && (depressAtime + lockout[MODE_INT] < now)) 
                                      || 
        ((hitOnTargB || hitOffTargB) && (depressBtime + lockout[MODE_INT] < now))) {
        lockedOut = true;
    } else {
      handleMode();
    }    
    }
  }
  if (ALRT_OffTarget) {
    long now = micros();
    //This bool is triggered by the ALRT_OffTarget_ISR with a window ALRT of 18000 - 32000 
    //A function that will do a thing
    if (!lockedOut) {
    if (((hitOnTargA || hitOffTargA) && (depressAtime + lockout[MODE_INT] < now)) 
                                      || 
        ((hitOnTargB || hitOffTargB) && (depressBtime + lockout[MODE_INT] < now))) {
        lockedOut = true;
    } else {
      handleMode();
    }    
    }
  }
  if (ALRT_Short) {
    //This bool is triggered by the ALRT_Short_ISR with a window ALRT of -10 - 10
    //A function that will do a thing
    handleMode();
  }
  if (modeChange) {
    //A function that will change the mode
    ChangeTheMODE();
    handleMode();

  }

  if (hitOnTargA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels , NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //Delay for a period of time (in milliseconds). Will be done In the Reset Function
    } 
  if (hitOffTargA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels, NUMPIXELS, CRGB::Yellow); // Yellow color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //Delay for a period of time (in milliseconds). Will be done In the Reset Function
    }  
  if (hitOnTargB) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels2, NUMPIXELS, CRGB::Red); // Moderately bright RED color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //Delay for a period of time (in milliseconds). Will be done In the Reset Function
    }
  if (hitOffTargB){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels2, NUMPIXELS, CRGB::Blue); // Bright Blue color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //Delay for a period of time (in milliseconds). Will be done In the Reset Function
    }
  //==========TESTING============
  String serData = String("hitOnTargA  : ") + hitOnTargA  + "\n"
                        + "hitOffTargA : "  + hitOffTargA + "\n"
                        + "hitOffTargB : "  + hitOffTargB + "\n"
                        + "hitOnTargB  : "  + hitOnTargB  + "\n"
                        + "Locked Out  : "  + lockedOut   + "\n";
  Serial.println(serData);
  //==========TESTING============
  if (lockedOut){
    resetValues();
    }
  delay(1);
}

