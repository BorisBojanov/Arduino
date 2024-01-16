#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
//TODO: set up debug levels correctly

//=======================
// Fast LED Setup
//=======================
  #define NUMPIXELS 64 // number of neopixels in strip
  #define DATA_PIN 12  // input pin Neopixel is attached to
  #define DATA_PIN2 19 // input for the seconf Neopixel LED
  #define CHIPSET WS2812B
  #define COLOR_ORDER GRB
  #define BRIGHTNESS 20
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
  Adafruit_ADS1115 ads1115_B;     // ADS1115 with ADDR pin connected to 3.3V (address 0x49)
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

  const int MODE_BUTTON_PIN = 13; // The pin where your mode button is connected
  const int DEBOUNCE_DELAY = 50; // The debounce delay in milliseconds
  unsigned long lastDebounceTime = 0; // The last time the button state changed  
  int lastButtonState = LOW; // The last known state of the button
  int buttonState;
  volatile bool ISR_StateChanged =false;


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
  int16_t Allowable_Deviation = 500;
  //Foil Values
  int16_t Foil_weapon_OffTarget_Threshold = 26000;
  int16_t Foil_lame_OffTarget_Threshold = 10;
  int16_t Foil_ground_OffTarget_Threshold = 10;

  int16_t Foil_weapon_OnTarget_Threshold = 8500;
  int16_t Foil_lame_OnTarget_Threshold = 8500;
  int16_t Foil_ground_OnTarget_Threshold = 8500;

  //Epee Values
  int16_t Epee_Weapon_OnTarget_Threshold = 13100;
  int16_t Epee_Lame_OnTarget_Threshold = 13100;
  int16_t Epee_Ground_OnTarget_Threshold = 13100;

  int16_t Epee_weapon_OffTarget_Threshold = 26000;
  int16_t Epee_lame_OffTarget_Threshold = 10000000;
  int16_t Epee_Ground_OffTarget_Threshold = 10696969696;

  //Sabre Values
  int16_t Sabre_weapon_OffTarget_Threshold = 26000;
  int16_t Sabre_lame_OffTarget_Threshold = 10;
  int16_t Sabre_ground_OffTarget_Threshold = 10;

  int16_t Sabre_weapon_OnTarget_Threshold = 8500;
  int16_t Sabre_lame_OnTarget_Threshold = 8500;
  int16_t Sabre_ground_OnTarget_Threshold = 8500;
//=======================
// depress and timeouts
//=======================
  long depressAtime = 0;
  long depressBtime = 0;
  bool lockedOut    = false;
//==========================
// states
//==========================
volatile boolean depressedA  = false;
volatile boolean depressedB  = false;
volatile boolean hitOnTargetA  = false;
volatile boolean hitOffTargetA = false;
volatile boolean hitOnTargetB  = false;
volatile boolean hitOffTargetB = false;
//==========================
//Forward Declare The functions used by struct WeaponMode
//==========================
void handleFoilHit();
void handleEpeeHit();
void handleSabreHit();
//==========================
// WeaponMode Changer
//==========================
volatile int32_t Foil_lockout = 300000;
volatile int32_t Epee_lockout = 45000;
volatile int32_t Sabre_lockout = 120000;

volatile int32_t Foil_Drepess = 14000;
volatile int32_t Epee_Depress = 0;
volatile int32_t Sabre_Depress = 10;

volatile struct WeaponMode {
  long lockoutTime;
  long depressTime;
  void (*handleHit)();
  int16_t weapon_OnTarget_Threshold;
  int16_t lame_OnTarget_Threshold;
  int16_t ground_OnTarget_Threshold;
  int16_t weapon_OffTarget_Threshold;
  int16_t lame_OffTarget_Threshold;
  int16_t ground_OffTarget_Threshold;
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

WeaponMode foilMode = {Foil_lockout, Foil_Drepess, handleFoilHit,
                       Foil_weapon_OnTarget_Threshold, Foil_lame_OnTarget_Threshold, Foil_ground_OnTarget_Threshold, 
                       Foil_weapon_OffTarget_Threshold, Foil_lame_OffTarget_Threshold, Foil_ground_OffTarget_Threshold};
WeaponMode epeeMode = {Epee_lockout, Epee_Depress, handleEpeeHit, 
                       Epee_Weapon_OnTarget_Threshold, Epee_Lame_OnTarget_Threshold, Epee_Ground_OnTarget_Threshold, 
                       Epee_weapon_OffTarget_Threshold, Epee_lame_OffTarget_Threshold, Epee_Ground_OffTarget_Threshold};
WeaponMode sabreMode = {Sabre_lockout, Sabre_Depress, handleSabreHit,
                       weapon_OnTarget_Threshold, lame_OnTarget_Threshold, ground_OnTarget_Threshold, 
                       weapon_OffTarget_Threshold, lame_OffTarget_Threshold, ground_OffTarget_Threshold};
WeaponMode* currentMode = &foilMode; // Default Mode

//Returns lockedOut = true if locked out
bool CheckForLockout(boolean& hitOnTargetA, boolean& hitOffTargetA, 
                     boolean& hitOnTargetB, boolean& hitOffTargetB, 
                     long& depressAtime, long& depressBtime, 
                     WeaponMode* currentMode) {
unsigned long now = micros();
//_____Check for lockout___
if(currentMode == &foilMode){
  if (
    ((hitOnTargetA || hitOffTargetA) && (depressAtime + currentMode->lockoutTime < now)) 
                                     ||
    ((hitOnTargetB || hitOffTargetB) && (depressBtime + currentMode->lockoutTime < now))) {
    lockedOut = true;
    return true;
  }
} else {
  return false;
  lockedOut = false;
  }
}


void handleHitWeapon(int16_t weaponA, int16_t lameA, int16_t weaponB, int16_t lameB, int32_t depressTime, 
  boolean& hitOnTarget, boolean& hitOffTarget, long& depressTimeVar, 
  WeaponMode* currentMode) {
  int16_t Allowable_Deviation = 500;
  unsigned long now = micros();
  if (!hitOnTargetA && !hitOffTargetA) {
    // Off target for WeaponA`
    if ((weaponA > (currentMode -> weapon_OffTarget_Threshold) && lameB < (currentMode -> lame_OffTarget_Threshold))
                                              ||
        (weaponB > weapon_OffTarget_Threshold && lameA < lame_OffTarget_Threshold)) {
      if (!depressedA) {
        unsigned long depressTimeVar = micros();
        depressedA = true;
      } else if (depressTimeVar + currentMode->depressTime <= now) {
        hitOffTargetA = true;
      }
    } else {
      // On target for WeaponA
      if ((weaponA > (currentMode -> weapon_OnTarget_Threshold - Allowable_Deviation) 
                                        && 
          weaponA < (currentMode -> weapon_OnTarget_Threshold + Allowable_Deviation)) 
                                                                        &&
                (lameB > (lame_OnTarget_Threshold - Allowable_Deviation) && lameB < (lame_OnTarget_Threshold + Allowable_Deviation))) {
        if (!depressedA) {
          unsigned long depressTimeVar = micros();
          depressedA = true;
        } else if (depressTimeVar + currentMode->depressTime <= now) {
          hitOnTargetA = true;
        }
      } else {
        depressTimeVar = 0;
        depressedA = false;
      }
    }
  }
}

void handleFoilHit() {
  // Serial.println("Inside: Handle Foil Function");
  volatile bool hitOnTargetA;
  volatile bool hitOffTargetA;
  volatile bool hitOnTargetB;
  volatile bool hitOffTargetB;
  volatile unsigned long depressAtime;
  volatile unsigned long depressBtime;
  volatile bool lockedOut;

  // read analog pins
  int16_t weaponA, lameA, groundA, adc3;
  int16_t weaponB, lameB, groundB, adc7;
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

  if(CheckForLockout(hitOnTargetA, hitOffTargetA, hitOnTargetB, hitOffTargetB, depressAtime, depressBtime, currentMode)) {
    handleHitWeapon();
    return; //returns to the main loop if locked out
  }

  
  unsigned long now = micros(); // Arduino uses millis() to get the number of milliseconds since the board started running.
                              //It's similar to the Python monotonic_ns() function but gives time in ms not ns.
  // ___Weapon B___
  if (!hitOnTargetB && !hitOffTargetB) {
      // Off target
      if (weaponB > weapon_OffTarget_Threshold && lameA < lame_OffTarget_Threshold) {
          if (!depressedB) {
              long depressBtime = micros();
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
                  long depressBtime = micros();
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
}

void handleEpeeHit() {
  // Serial.println("Inside: Handle EPEE Function");
  // Implement the specific behavior for a hit in Epee mode
  // read analog pins
    int16_t weaponA, lameA, groundA, adc3;
    int16_t weaponB, lameB, groundB, adc7;
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
      if (weaponA > (Epee_Weapon_OnTarget_Threshold - Allowable_Deviation) && weaponA < (Epee_Weapon_OnTarget_Threshold + Allowable_Deviation)
                                                                      && 
              lameA > (Epee_Lame_OnTarget_Threshold - Allowable_Deviation) && lameA < (Epee_Lame_OnTarget_Threshold + Allowable_Deviation)) {
          if (!depressedA) {
            long depressAtime = micros();
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
      if (weaponB > (Epee_Weapon_OnTarget_Threshold - Allowable_Deviation) && weaponB < (Epee_Weapon_OnTarget_Threshold + Allowable_Deviation)
                                                                      && 
              lameB > (Epee_Lame_OnTarget_Threshold - Allowable_Deviation) && lameB < (Epee_Lame_OnTarget_Threshold + Allowable_Deviation)) {
          if (!depressedB) {
            long depressBtime = micros();
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
  // Serial.println("Inside: Handle Sabre Function");
  // Implement the specific behavior for a hit in Sabre mode
  // read analog pins
    int16_t weaponA, lameA, groundA, adc3;
    int16_t weaponB, lameB, groundB, adc7;
    weaponA = ads1115_A.readADC_SingleEnded(0);    // Read from channel 0 of the first ADS1115
    lameA = ads1115_A.readADC_SingleEnded(1);    // Read from channel 1 of the first ADS1115
    groundA = ads1115_A.readADC_SingleEnded(2);    // Read from channel 2 of the first ADS1115

    weaponB = ads1115_B.readADC_SingleEnded(0);   // Read from channel 0 of the second ADS1115
    lameB = ads1115_B.readADC_SingleEnded(1);   // Read from channel 1 of the second ADS1115
    groundB = ads1115_B.readADC_SingleEnded(2);   // Read from channel 2 of the second ADS1115
  //   Serial.print("Default ADS1115 (0x48) weaponA 0: "); Serial.println(weaponA);
  //   Serial.print("Default ADS1115 (0x48) lameA 1: "); Serial.println(lameA);
  //   Serial.print("Default ADS1115 (0x48) groundA 2: "); Serial.println(groundA);
    
  //   Serial.print("ADDR ADS1115 (0x49) weaponB 4: "); Serial.println(weaponB);
  //   Serial.print("ADDR ADS1115 (0x49) lameB 5: "); Serial.println(lameB);
  //   Serial.print("ADDR ADS1115 (0x49) groundB 6: "); Serial.println(groundB);
  // //_____Check for lockout___
   long now = micros();
   if (((hitOnTargetA) && (depressAtime + sabreMode.lockoutTime < now)) 
                                        || 
       ((hitOnTargetB) && (depressBtime + sabreMode.lockoutTime < now))) {
      lockedOut = true;
   }

  // weapon A
 if (!hitOnTargetA && !hitOffTargetA) {
      // Off target
      if (weaponA > weapon_OffTarget_Threshold && lameB < lame_OffTarget_Threshold) {
          if (!depressedA) {
              long depressAtime = micros();
              depressedA = true;
          } else if (depressAtime + foilMode.depressTime <= now) {
              hitOffTargetA = true;
          }
      } else {
      // on target
      if (weaponA > (lame_OnTarget_Threshold - Allowable_Deviation) && weaponA < (lame_OnTarget_Threshold + Allowable_Deviation) 
                                                                    && 
            lameB > (lame_OnTarget_Threshold - Allowable_Deviation) && lameB < (lame_OnTarget_Threshold + Allowable_Deviation)) {
         if (!depressedA) {
            long depressAtime = micros();
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
 }
  // weapon B
  if (!hitOnTargetB && !hitOffTargetB) {
      // Off target
      if (weaponB > weapon_OffTarget_Threshold && lameA < lame_OffTarget_Threshold) {
          if (!depressedB) {
              long depressBtime = micros();
              depressedB = true;
          } else if (depressBtime + foilMode.depressTime <= now) {
              hitOffTargetB = true;
          }
      } else {
      // on target
      if (weaponB > (lame_OnTarget_Threshold - Allowable_Deviation) && weaponB < (lame_OnTarget_Threshold + Allowable_Deviation) 
                                                                    &&
            lameA > (lame_OnTarget_Threshold - Allowable_Deviation) && lameA < (lame_OnTarget_Threshold + Allowable_Deviation)) {
         if (!depressedB) {
            long depressBtime = micros();
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
}

void handleHit() {
    currentMode->handleHit(); // will point to a Mode instance in struct Mode
}

void modeChange() { 
  buttonState = LOW;
  if (currentMode == &foilMode) {
    currentMode = &epeeMode;
  } else if (currentMode == &epeeMode) {
    currentMode = &sabreMode;
  } else {
    currentMode = &foilMode;
  }
}

void BUTTON_ISR() {
  ISR_StateChanged = true;
}

bool BUTTON_Debounce(int Some_Button_PIN){
  int reading = digitalRead(Some_Button_PIN);
  static int buttonState;
  static int lastButtonState = LOW;
  static unsigned long lastDebounceTime = 0;
  int DEBOUNCE_DELAY = 50; // Adjust this value based on your specific button and requirements

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH
      if (buttonState == HIGH) {
        Serial.println("changed mode was called");
        return true;
      }
    }
  }
  lastButtonState = reading;
  // Return false if no change or if not meeting the conditions
  return false;
}

//======================
// Reset all variables
//======================
unsigned long lastResetTime = 0;
void resetValues() {
  unsigned long currentMillis = millis(); // get the current time
  if (currentMillis - lastResetTime >= LIGHTTIME) { // check if delayTime has passed
    // wait before turning off the buzzer
    digitalWrite(buzzerPin,  LOW);
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

    lastResetTime = currentMillis; // save the last time you reset the values
  }
}
void setup() {
  Serial.begin(BAUDRATE);
  Serial.println("Score_box_V4_BetterModeChange");
  pinMode(MODE_BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(buzzerPin, OUTPUT);
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
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(pixels, NUMPIXELS);
  FastLED.addLeds<CHIPSET, DATA_PIN2, COLOR_ORDER>(pixels222, NUMPIXELS);
  FastLED.setBrightness(BRIGHTNESS);
  //attatch the interrupts we need to check
  //attachInterrupt(digitalPinToInterrupt(ads1115_A.readADC_SingleEnded(0)), handleHit, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(ads1115_B.readADC_SingleEnded(0)), handleHit, CHANGE); //SEt a different interrupt PIN
  attachInterrupt(digitalPinToInterrupt(MODE_BUTTON_PIN), BUTTON_ISR, FALLING);
  resetValues();
}

void loop() {
  volatile bool hitOnTargetA;
  volatile bool hitOffTargetA;
  volatile bool hitOnTargetB;
  volatile bool hitOffTargetB;
  volatile unsigned long depressAtime;
  volatile unsigned long depressBtime;
  volatile bool lockedOut;
  //main code here, to run repeatedly:
  if(BUTTON_Debounce(MODE_BUTTON_PIN)){
    modeChange();
  }

if(!lockedOut) {
handleHit(); // will point to a Mode instance in struct Mode Foil,Epee, or Sabre
}
 
CheckForLockout(hitOnTargetA, hitOffTargetA, hitOnTargetB, hitOffTargetB, depressAtime, depressBtime, currentMode); // This is either true or False

//maybe want to make a reset function for each mode
if ( lockedOut|| (currentMode == &sabreMode && (hitOffTargetB || hitOffTargetA))){
  resetValues();
  }

  if (hitOnTargetA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels222, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    digitalWrite(buzzerPin, HIGH);
    }
  if (hitOffTargetA) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels222, NUMPIXELS, CRGB::Yellow); // Yellow color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    digitalWrite(buzzerPin, HIGH);
    }
  if (hitOnTargetB) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels, NUMPIXELS, CRGB::Red); // Moderately bright RED color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    digitalWrite(buzzerPin, HIGH);
    }
  if (hitOffTargetB){
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    fill_solid(pixels, NUMPIXELS, CRGB::Blue); // Bright Blue color.
    FastLED.show(); // This sends the updated pixel color to the hardware.
    digitalWrite(buzzerPin, HIGH);
    }

  // String serData = String("hitOnTargetA  : ") + hitOnTargetA  + "\n"
  //                       + "hitOffTargetA : "  + hitOffTargetA + "\n"
  //                       + "hitOffTargetB : "  + hitOffTargetB + "\n"
  //                       + "hitOnTargetB  : "  + hitOnTargetB  + "\n"
  //                       + "Locked Out  : "  + lockedOut   + "\n";
  //   Serial.println(serData);
}
 