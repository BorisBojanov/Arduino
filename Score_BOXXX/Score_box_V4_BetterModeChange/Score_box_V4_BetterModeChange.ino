#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
//TODO: set up debug levels correctly
  #define DEBUG 0
//=======================
// Fast LED Setup
//=======================
  #define NUMPIXELS 64 // number of neopixels in strip
  struct CRGB pixels[NUMPIXELS];
  struct CRGB pixels222[NUMPIXELS];
//=======================
// ADC Defines and info
//=======================
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

  Adafruit_ADS1115 ads1115_A;  // ADS1115 with ADDR pin floating (default address 0x48)
  Adafruit_ADS1115 ads1115_B;     // ADS1115 with ADDR pin connected to 3.3V
  const int threshold = 400; // The threshold for triggering the interrupt
  int previousValueA = 0;
  int previousValueB = 0;
//============
// Pin Setup
// Gpios 6, 7, 8, 9, 10 and 11, 12 are a no go for ESP32 to use
// Gpio 0 is always high at 4096
// Gpio 35 is always high at 4096
//============
  #define BUZZERTIME  1000  // length of time the buzzer is kept on after a hit (ms)
  #define LIGHTTIME   3000  // length of time the lights are kept on after a hit (ms)
  #define BAUDRATE   115200  // baudrate of the serial debug interface
  int delayval = 100; // timing delay in milliseconds
  const int MODE_BUTTON_PIN = 25; // The pin where your mode button is connected
  const int DEBOUNCE_DELAY = 1000; // The debounce delay in milliseconds
  int lastButtonState = LOW; // The last known state of the button
  unsigned long lastDebounceTime = 0; // The last time the button state changed  
  const uint8_t PIN = 27;    // input pin Neopixel is attached to
  const uint8_t PIN222 = 26; // input for the seconf Neopixel LED
  const uint8_t buzzerPin= 5;    // buzzer pin
//=========================
// values of analog reads
// Note: volatile is used to have the variable updated more frequently 
//       as the code is run,
//=========================
  volatile int weaponA = 0;
  volatile int weaponB = 0;
  volatile int lameA   = 0;
  volatile int lameB   = 0;
  volatile int groundA = 0;
  volatile int groundB = 0;

  // for all X: weapon >= Values >= lame  
  int weapon_OffTarget_Threshold = 26000;
  int lame_OffTarget_Threshold = 10;
  int ground_OffTarget_Threshold = 10;

  int Allowable_Deviation = 500;
  int weapon_OnTarget_Threshold = 8500;
  int lame_OnTarget_Threshold = 8500;
  int ground_OnTarget_Threshold = 8500;

//=======================
// depress and timeouts
//=======================
  long depressAtime = 0;
  long depressBtime = 0;
  bool lockedOut    = false;
//==========================
// states
//==========================
  bool depressedA  = false;
  bool depressedB  = false;
  bool hitOnTargetA  = false;
  bool hitOffTargetA = false;
  bool hitOnTargetB  = false;
  bool hitOffTargetB = false;
//==========================
//Forward Declare The functions used by struct Mode
//==========================
void handleFoilHit();
void handleEpeeHit();
void handleSabreHit();
//==========================
// WeaponMode Changer
//==========================
struct WeaponMode {
  int lockoutTime;
  int depressTime;
  void (*handleHit)();
};
//==========================
// Weapon Specifiv Code
//==========================
// Lockout & Depress Times
//==========================
// the lockout time between hits for FOIL is 300ms +/-25ms
// the minimum amount of time the tip needs to be depressed for FOIL 14ms +/-1ms
// the lockout time between hits for EPEE is 45ms +/-5ms (40ms -> 50ms)
// the minimum amount of time the tip needs to be depressed for EPEE 2ms
// the lockout time between hits for SABRE is 120ms +/-10ms
// the minimum amount of time the tip needs to be depressed for SABRE 0.1ms -> 1ms
// These values are stored as micro seconds for more accuracy
WeaponMode foilMode = {300000, 14000, handleFoilHit};
WeaponMode epeeMode = {45000, 2000, handleEpeeHit};
WeaponMode sabreMode = {120000, 1000, handleSabreHit};
WeaponMode* currentMode = &foilMode; // Default Mode


void handleFoilHit() {
  Serial.println("Inside: Handle Foil Function");
  // Implement the specific behavior for a hit in Foil mode
  // read analog pins
    weaponA = ads1115_A.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
    lameA = ads1115_A.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
    groundA = ads1115_A.readADC_SingleEnded(2);    // Read from channel 2 of the first ADS1115

    weaponB = ads1115_B.readADC_SingleEnded(0);   // Read from channel 0 of the second ADS1115
    lameB = ads1115_B.readADC_SingleEnded(1);   // Read from channel 1 of the second ADS1115
    groundB = ads1115_B.readADC_SingleEnded(2);   // Read from channel 2 of the second ADS1115
    // Serial.print("Default ADS1115 (0x48) weaponA 0: "); Serial.println(weaponA);
    // Serial.print("Default ADS1115 (0x48) lameA 1: "); Serial.println(lameA);
    // Serial.print("Default ADS1115 (0x48) groundA 2: "); Serial.println(groundA);
    
    // Serial.print("ADDR ADS1115 (0x49) weaponB 4: "); Serial.println(weaponB);
    // Serial.print("ADDR ADS1115 (0x49) lameB 5: "); Serial.println(lameB);
    // Serial.print("ADDR ADS1115 (0x49) groundB 6: "); Serial.println(groundB);
    

    long now = millis(); // Arduino uses millis() to get the number of milliseconds since the board started running.
                                //It's similar to the Python monotonic_ns() function but gives time in ms not ns.
  Serial.print(depressAtime + foilMode.lockoutTime); Serial.print(" "); Serial.println(now);
  //_____Check for lockout___
  if (((hitOnTargetA || hitOffTargetA) && (depressAtime + foilMode.lockoutTime < now)) ||
          ((hitOnTargetB || hitOffTargetB) && (depressBtime + foilMode.lockoutTime < now))) {
      lockedOut = true;
  }

  // ___Weapon A___
  if (!hitOnTargetA && !hitOffTargetA) {
      // Off target
      if (weaponA > weapon_OffTarget_Threshold && lameB < lame_OffTarget_Threshold) {
          if (!depressedA) {
              depressAtime = micros();
              depressedA = true;
          } else if (depressAtime + foilMode.depressTime <= now) {
              hitOffTargetA = true;


          }
      } else {
          // On target
          if (weaponA > (weapon_OnTarget_Threshold - Allowable_Deviation) && weaponA < (weapon_OnTarget_Threshold + Allowable_Deviation) 
                                                                          && 
                  lameB > (lame_OnTarget_Threshold - Allowable_Deviation) && lameB < (lame_OnTarget_Threshold + Allowable_Deviation)) {
              if (!depressedA) {
                  depressAtime = micros();
                  depressedA = true;
              } else if (depressAtime + foilMode.depressTime <= now) {
                  hitOnTargetA = true;
              }
          } else {
              depressAtime = 0;
              depressedA = false;
          }
      }
  }

  // ___Weapon B___
  if (!hitOnTargetB && !hitOffTargetB) {
      // Off target
      if (weaponB > weapon_OffTarget_Threshold && lameA < lame_OffTarget_Threshold) {
          if (!depressedB) {
              depressBtime = micros();
              depressedB = true;
          } else if (depressBtime + foilMode.depressTime <= now) {
              hitOffTargetB = true;
          }
      } else {
          // On target
          if (weaponB > (weapon_OnTarget_Threshold - Allowable_Deviation) && weaponB < (weapon_OnTarget_Threshold + Allowable_Deviation)
                                                                          && 
                  lameA > (lame_OnTarget_Threshold - Allowable_Deviation) && lameA < (lame_OnTarget_Threshold + Allowable_Deviation)) {
              if (!depressedB) {
                  depressBtime = micros();
                  depressedB = true;
              } else if (depressBtime + foilMode.depressTime <= now) {
                  hitOnTargetB = true;
              }
          } else {
              depressBtime = 0;
              depressedB = false;
          }
      }
  }

  if (lockedOut){
    
    resetValues();
    }
  delay(1);
}

void handleEpeeHit() {
  Serial.println("Inside: Handle EPEE Function");
  // Implement the specific behavior for a hit in Epee mode
  // read analog pins
    weaponA = ads1115_A.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
    lameA = ads1115_A.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
    groundA = ads1115_A.readADC_SingleEnded(2);    // Read from channel 2 of the first ADS1115

    weaponB = ads1115_B.readADC_SingleEnded(0);   // Read from channel 0 of the second ADS1115
    lameB = ads1115_B.readADC_SingleEnded(1);   // Read from channel 1 of the second ADS1115
    groundB = ads1115_B.readADC_SingleEnded(2);   // Read from channel 2 of the second ADS1115
    
    //_____Check for lockout___
    long now = micros();
    if ((hitOnTargetA && (depressAtime + epeeMode.lockoutTime < now)) 
      ||
      (hitOnTargetB && (depressBtime + epeeMode.lockoutTime < now))) {
      lockedOut = true;
    }

    // weapon A
    //  no hit for A yet    && weapon depress    && opponent lame touched
    if (hitOnTargetA == false) {
      if (400 < weaponA && weaponA < 600 && 400 < lameA && lameA < 600) {
          if (!depressedA) {
            depressAtime = micros();
            depressedA   = true;
          } else {
            if (depressAtime + epeeMode.depressTime <= micros()) {
                hitOnTargetA = true;
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
    if (hitOnTargetB == false) {
      if (400 < weaponB && weaponB < 600 && 400 < lameB && lameB < 600) {
          if (!depressedB) {
            depressBtime = micros();
            depressedB   = true;
          } else {
            if (depressBtime + epeeMode.depressTime <= micros()) {
                hitOnTargetB = true;
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

void handleSabreHit() {
  Serial.println("Inside: Handle Sabre Function");
  // Implement the specific behavior for a hit in Sabre mode
  // read analog pins
    int16_t adc0, adc1, adc2, adc3;
    int16_t adc4, adc5, adc6, adc7;

    float volts0, volts1, volts2, volts3;
    float volts4, volts5, volts6, volts7;

    weaponA = ads1115_A.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
    lameA = ads1115_A.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
    groundA = ads1115_A.readADC_SingleEnded(2);    // Read from channel 2 of the first ADS1115

    weaponB = ads1115_B.readADC_SingleEnded(0);   // Read from channel 0 of the second ADS1115
    lameB = ads1115_B.readADC_SingleEnded(1);   // Read from channel 1 of the second ADS1115
    groundB = ads1115_B.readADC_SingleEnded(2);   // Read from channel 2 of the second ADS1115
  //_____Check for lockout___
   long now = micros();
   if (((hitOnTargetA || hitOffTargetA) && (depressAtime + sabreMode.lockoutTime < now)) || 
       ((hitOnTargetB || hitOffTargetB) && (depressBtime + sabreMode.lockoutTime < now))) {
      lockedOut = true;
   }

  // weapon A
   if (hitOnTargetA == false && hitOffTargetA == false) { // ignore if A has already hit
      // on target
      if (400 < weaponA && weaponA < 600 && 400 < lameB && lameB < 600) {
         if (!depressedA) {
            depressAtime = micros();
            depressedA   = true;
         } else {
            if (depressAtime + sabreMode.depressTime <= micros()) {
               hitOnTargetA = true;
            }
         }
      } else {
         // reset these values if the depress time is short.
         depressAtime = 0;
         depressedA   = 0;
      }
   }

  // weapon B
   if (hitOnTargetB == false && hitOffTargetB == false) { // ignore if B has already hit
      // on target
      if (400 < weaponB && weaponB < 600 && 400 < lameA && lameA < 600) {
         if (!depressedB) {
            depressBtime = micros();
            depressedB   = true;
         } else {
            if (depressBtime + sabreMode.depressTime <= micros()) {
               hitOnTargetB = true;
            }
         }
      } else {
         // reset these values if the depress time is short.
         depressBtime = 0;
         depressedB   = 0;
      }
   }
}

void handleHit() {
    currentMode->handleHit(); // will pint to a Mode instance in struct Mode
}

void modeChangeISR() { 
  if (currentMode == &foilMode) {
    currentMode = &epeeMode;
  } else if (currentMode == &epeeMode) {
    currentMode = &sabreMode;
  } else {
    currentMode = &foilMode;
  }
}

void BUTTON_Debounce(){
  //BUTTON Debounce
  int Mode_reading = digitalRead(MODE_BUTTON_PIN);
  if (Mode_reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) >= DEBOUNCE_DELAY) {
    if (Mode_reading != digitalRead(MODE_BUTTON_PIN)) {
      lastButtonState = Mode_reading;

      if (Mode_reading == HIGH) {
        // The button was just pressed, change the mode
        modeChangeISR();
      }
    }
  }
}
//======================
// Reset all variables
//======================
void resetValues() {
  delay(BUZZERTIME);             // wait before turning off the buzzer
  //digitalWrite(buzzerPin,  LOW);
  delay(LIGHTTIME-BUZZERTIME);   // wait before turning off the lights
  //Removed this from all Fill color commands and placed here to see if this command is being excicuted
  fill_solid(pixels, NUMPIXELS, CRGB::Black);
  fill_solid(pixels222, NUMPIXELS, CRGB::Black);
  FastLED.show();
  lockedOut    = false;
  depressedA   = false;
  depressedB   = false;
  hitOnTargetA  = false;
  hitOffTargetA = false;
  hitOnTargetB  = false;
  hitOffTargetB = false;
  depressBtime = 0;
  depressAtime = 0;
  delay(5);
}
void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Score_box_V4_BetterModeChange");
  pinMode(MODE_BUTTON_PIN, INPUT_PULLDOWN);
  //  Serial.println("Three Weapon Scoring Box");
  //  Serial.println("================");
  //=========
  //ADC SETUP
  //=========
  ads1115_A.begin(0x48);    // Initialize the first ADS1115 at default address 0x48 (ADDR is a Floating Pin)
  ads1115_B.begin(0x49);       // Initialize the second ADS1115 at address 0x49 (ADDR connected to VDD aka power of ADC Pin)

  //====
  while (!Serial); // Wait for the serial port to connect
  if (!ads1115_A.begin(0x48)) {
    Serial.println("Failed to initialize ADS1115 at default 0x48!");
  }
  if (!ads1115_B.begin(0x49)) {
    Serial.println("Failed to initialize ADS1115 at address 0x49!");
  }
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC GAIN_ONE Range: +/- 4.096V (1 bit = 0.125mV/ADS1115)");
  //====
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
  ads1115_A.setGain(GAIN_ONE);
  ads1115_B.setGain(GAIN_ONE);
  //FAST LED SETUP
  FastLED.addLeds<WS2812B, PIN, GRB>(pixels, NUMPIXELS);
  FastLED.addLeds<WS2812B, PIN222, GRB>(pixels222, NUMPIXELS);
  
  //attatch the interrupts we need to check
  //attachInterrupt(digitalPinToInterrupt(ads1115_A.readADC_SingleEnded(0)), handleHit, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(ads1115_B.readADC_SingleEnded(0)), handleHit, CHANGE); //SEt a different interrupt PIN
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON_PIN), BUTTON_Debounce, FALLING);
  resetValues();
}

void loop() {
  int16_t weaponA, lameA, groundA, adc3;
  int16_t weaponB, lameB, groundB, adc7;
  weaponA = ads1115_A.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
  lameA = ads1115_A.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
  groundA = ads1115_A.readADC_SingleEnded(2);    // Read from channel 2 of the first ADS1115

  weaponB = ads1115_B.readADC_SingleEnded(0);   // Read from channel 0 of the second ADS1115
  lameB = ads1115_B.readADC_SingleEnded(1);   // Read from channel 1 of the second ADS1115
  groundB = ads1115_B.readADC_SingleEnded(2);   // Read from channel 2 of the second ADS1115

  handleHit();
  if (hitOnTargetA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels222, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
    } else {
    // fill_solid(pixels222, NUMPIXELS, CRGB::Black);
    // FastLED.show();
    //delay(delayval);
    }
  if (hitOffTargetA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels222, NUMPIXELS, CRGB::Yellow); // Yellow color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
    } else {
    // fill_solid(pixels222, NUMPIXELS, CRGB::Black);
    // FastLED.show();
    //delay(delayval
    }
  if (hitOnTargetB) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels, NUMPIXELS, CRGB::Red); // Moderately bright RED color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
    } else {
    // fill_solid(pixels, NUMPIXELS, CRGB::Black);
    // FastLED.show();
    // delay(delayval);
    }
  if (hitOffTargetB){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels, NUMPIXELS, CRGB::Blue); // Bright Blue color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    //delay(delayval); // Delay for a period of time (in milliseconds).
    } else {
    // fill_solid(pixels, NUMPIXELS, CRGB::Black);
    // FastLED.show();
    // delay(delayval);
    }
  String serData = String("hitOnTargetA  : ") + hitOnTargetA  + "\n"
                        + "hitOffTargetA : "  + hitOffTargetA + "\n"
                        + "hitOffTargetB : "  + hitOffTargetB + "\n"
                        + "hitOnTargetB  : "  + hitOnTargetB  + "\n"
                        + "Locked Out  : "  + lockedOut   + "\n";
  Serial.println(serData);
  BUTTON_Debounce();
  if (lockedOut){

    resetValues();
    }
  delay(1);
}
 