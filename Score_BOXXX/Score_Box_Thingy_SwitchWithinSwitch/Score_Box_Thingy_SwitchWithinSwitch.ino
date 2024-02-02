//Scorebox thingy with switch case(Foil, Epee, Sabre) leading into switch case(OnTargetA, OffTargetA, OnTargetB, OffTargetB)
/*
TODO:
- potentially remove Depress Time from prjoect
- add a way to reset
    - reset the lockout timer
    - reset the depress timer
- function for setting a lockedOut bool value and checking if the lockedOut value has changed
    - this then calls the reset function once lockedOut == true
*/

#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <FastLED.h>

//============
// Pin Setup
// Gpios 6, 7, 8, 9, 10 and 11, 12 are a no go for ESP32 to use
// Gpio 0 is always high
// Gpio 35 is always high
//============
#define BUZZERTIME  1000  // length of time the buzzer is kept on after a hit (ms)
#define LIGHTTIME   3000  // length of time the lights are kept on after a hit (ms)
#define BAUDRATE   921600  // baudrate of the serial debug interface
#define MODE_BUTTON_PIN 23
#define BUZZER_PIN 5
int WeaponA_ONTARGET_ALRT_PIN = 13;
int WeaponB_ONTARGET_ALRT_PIN = 14;
int WeaponA_OFFTARGET_ALRT_PIN =15;
int WeaponB_OFFTARGET_ALRT_PIN =18;

//LEDS
#define NUMPIXELS 64 // number of neopixels in strip
#define DATA_PIN 12  // input pin Neopixel is attached to
#define DATA_PIN2 19 // input for the seconf Neopixel LED
#define BRIGHTNESS 20 // 0-255, higher number is brighter
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
struct CRGB pixelsl[NUMPIXELS];
struct CRGB pixelsr[NUMPIXELS];

//Global Variables
int lastButtonState = LOW; // The last known state of the button
int buttonState;
unsigned long lastResetTime    = 0; // The last time in millis that the reset was called
volatile bool ISR_StateChanged = false; // This is the flag that is set when the button is pressed
volatile bool OnTargetA_Flag   = false;
volatile bool OnTargetB_Flag   = false;
volatile bool OffTargetA_Flag  = false;
volatile bool OffTargetB_Flag  = false;


volatile int32_t timeHitWasMade = 0; // This is the time the hit was made

Adafruit_ADS1115 ads1115_weaponONTARGET_A_B;  // ADS1115 with ADDR pin floating (default address 0x48)
Adafruit_ADS1115 ads1115_weaponOFFTARGET_A_B;     // ADS1115 with ADDR pin connected to 3.3V (address 0x49)

// Global structs to store the current weapon mode
struct WeaponMode {
    //Variable declaration for the switch cases
    long lockoutTime;
    //long depressTime;
    int Mode_INT; //used by the switch statement to select weaponMode
};

// struct HitType {
//     int OnTargetA;
//     int OffTargetA;
//     int OnTargetB;
//     int OffTargetB;
// };


struct ADC_OnTarget_readings {
    //=============ADS_with On target threshold=======
    uint32_t weapon_ADS_OnTarget; // WeaponA or WeaponB
    uint32_t lame_ADS_OnTarget; // LameA or LameB
    int HitTypeINT; // 1 for OnTargetA, 2 for OffTargetA, 3 for OnTargetB, 4 for OffTargetB
};

struct ADC_Differentials {
    //=============ADS_with On target or Off Target=======
    uint32_t weaponA_Diff_lameB; // WeaponA - LameB : for foil and Sabre only
    uint32_t weaponB_Diff_lameA; // WeaponB - LameA : for foil and Sabre only
    int HitTypeINT; // 1 for OnTargetA, 2 for OffTargetA
};

//Build the structs that will be used in the functions
const uint16_t Foil_lockout  = 300; // 300,000 microseconds = 300 milliseconds
const uint16_t Epee_lockout  = 45;  // 45,000 microseconds = 45 milliseconds
const u_int16_t Sabre_lockout = 170; //170,000 microseconds = 170 milliseconds

// const u_int8_t Foil_Drepess  = 20;    // 2000 microseconds = 2 milliseconds
// const u_int8_t Epee_Depress  = 10;    // 10 microseconds = .01 milliseconds
// const u_int8_t Sabre_Depress = 10;    // 10 microseconds = .01 milliseconds
// // for all X: weapon >= Values >= lame  
// const int16_t Allowable_Deviation = 500;
// //Foil/Sabre Values
// const u_int16_t weapon_OffTarget_Threshold = 26000;
// const u_int16_t lame_OffTarget_Threshold = 10;
// const u_int16_t ground_OffTarget_Threshold = 10;

// const u_int16_t weapon_OnTarget_Threshold = 13100;
// const u_int16_t lame_OnTarget_Threshold = 13100;
// const u_int16_t ground_OnTarget_Threshold = 0;
// //Epee Values
// const u_int16_t Epee_Weapon_OnTarget_Threshold = 13100;
// const u_int16_t Epee_Lame_OnTarget_Threshold = 13100;
// const u_int16_t Epee_Ground_OnTarget_Threshold = 13100;

const u_int16_t weaponA = 0, lameA = 0, weaponB = 0, lameB = 0;

/*
//=============ADS_with On target threshold=======
uint32_t weaponA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0); // WeaponA
uint32_t lameB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(1); // LameB
uint32_t weaponB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(2); // WeaponB
uint32_t lameA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(3); // LameA
//=============ADS_with Off target threshold=======
uint32_t weaponA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(0); // WeaponA
uint32_t lameB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(1); // LameB
uint32_t weaponB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(2); // WeaponB
uint32_t lameA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(3); // LameA

//=============ADS_with On target threshold=======
uint32_t weaponA_Diff_lameB_OnTarget =ads1115_weaponONTARGET_A_B.readADC_Differential_0_1(); // WeaponA - LameB : for foil and Sabre only
uint32_t weaponB_Diff_lameA_OnTarget =ads1115_weaponONTARGET_A_B.readADC_Differential_2_3(); // WeaponB - LameA : for foil and Sabre only
//=============ADS_with Off target threshold=======
uint32_t weaponA_Diff_lameB_OffTarget =ads1115_weaponOFFTARGET_A_B.readADC_Differential_0_1(); // WeaponA - LameB : for foil and Sabre only
uint32_t weaponB_Diff_lameA_OffTarget =ads1115_weaponOFFTARGET_A_B.readADC_Differential_2_3(); // WeaponB - LameA : for foil and Sabre only
*/


//Build the structs     long           int
WeaponMode foilMode =  {Foil_lockout,   1};
WeaponMode epeeMode =  {Epee_lockout,   2};
WeaponMode sabreMode = {Sabre_lockout,  3};
WeaponMode* currentMode = &foilMode; // Default Mode and Initializing the cariable currentMode
/*
    //=============ADS_with On target threshold=======
    uint16_t weaponA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(3); // LameA
    //=============ADS_with Off target threshold=======
    uint16_t weaponA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(3); // LameA
*/
uint16_t weaponA_ADS_OnTarget;
uint16_t lameB_ADS_OnTarget;
uint16_t weaponB_ADS_OnTarget;
uint16_t lameA_ADS_OnTarget;
uint16_t weaponA_ADS_OffTarget;
uint16_t lameB_ADS_OffTarget;
uint16_t weaponB_ADS_OffTarget;
uint16_t lameA_ADS_OffTarget;

ADC_OnTarget_readings OnTargetA  = {weaponA_ADS_OnTarget, lameB_ADS_OnTarget, 6 };
ADC_OnTarget_readings OnTargetB  = {weaponB_ADS_OnTarget, lameA_ADS_OnTarget, 7};
ADC_OnTarget_readings OffTargetA = {weaponA_ADS_OffTarget, lameB_ADS_OffTarget, 8 };
ADC_OnTarget_readings OffTargetB = {weaponB_ADS_OffTarget, lameA_ADS_OffTarget, 9};
ADC_OnTarget_readings NOhitsssss = {0, 0, 0};                                       // No hits in a frame
ADC_OnTarget_readings *HitType = &NOhitsssss; // Make HitType point to OnTargetA
int HitTypes = HitType -> HitTypeINT; // Now this line is safe

//These functions check the depress time and then set the HitTypeINT to the correct value
void Ident_OnTargetA(ADC_OnTarget_readings* newHitType){
    /*
    currentMode is being declared like this since we are referenceing a value within foilMode, EpeeMode, or Sabre Mode
        WeaponMode currentMode
    if we wanted to refference the mode itself and not a value within the mode we would say
        WeaponMode *currentMode
    _____________________
    Weapon Mode dependant
    Check depress time, basically a weapon dependant Button debounce.
    return
    HitType = 1
    aka
    HitType = Ident_OnTargetA
    */
   Serial.println("Hello THERE general Kenobi");
    HitType = newHitType; // Set the HitType to the newHitTypes struct passed in from main loop
}
void Ident_OnTargetB (ADC_OnTarget_readings* newHitType){
HitType = newHitType; // Set the HitType to the newHitTypes struct passed in from main loop
}
void Ident_OffTargetA(ADC_OnTarget_readings* newHitType){
HitType = newHitType; // Set the HitType to the newHitTypes struct passed in from main loop
}
void Ident_OffTargetB(ADC_OnTarget_readings* newHitType){    
HitType = newHitType; // Set the HitType to the newHitTypes struct passed in from main loop
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

// HitType OnTargetA  = {1, 0, 0, 0};
// HitType OffTargetA = {0, 2, 0, 0};
// HitType OnTargetB  = {0, 0, 3, 0};
// HitType OffTargetB = {0, 0, 0, 4};

//each one of these takes the value of the Weapon from that side and then compares it to the lame of the other(with exception of Epee)
//if the value is greater than the threshold then it is a hit
// ADC_OnTarget_readings OnTargetA = {long WeaponA,                                     long LameB,                                  int HitType OnTargetA }



// Assign each flag a unique power of 2
const int OnTargetA_Flag_Value = 1; // 2^0
const int OnTargetB_Flag_Value = 2; // 2^1
const int OffTargetA_Flag_Value = 4; // 2^2
const int OffTargetB_Flag_Value = 8; // 2^3

// Calculate the state of the flags
int flagState = (OnTargetA_Flag * OnTargetA_Flag_Value) +
                (OnTargetB_Flag * OnTargetB_Flag_Value) +
                (OffTargetA_Flag * OffTargetA_Flag_Value) +
                (OffTargetB_Flag * OffTargetB_Flag_Value);

void ISR_OnTargetA(){//This function is potentially being called very often 
    timeHitWasMade = micros();
    OnTargetA_Flag = weaponA_ADS_OnTarget - lameB_ADS_OnTarget < 1000;
}
void ISR_OnTargetB(){
    timeHitWasMade = micros();
    OnTargetB_Flag = true;
}
void ISR_OffTargetA(){
    timeHitWasMade = micros();
    OffTargetA_Flag = true;
}
void ISR_OffTargetB(){
    timeHitWasMade = micros();
    OffTargetB_Flag = true;
}

void BUTTON_ISR() {
  ISR_StateChanged = true;
}
/*
This function is used to debounce any button, taking an INT of the button PIN that we want debounced
BUTTON_Debounce(MODE_BUTTON_PIN) returns a boolean value
in a loop we would use it like this
  if(BUTTON_Debounce(MODE_BUTTON_PIN)){
    modeChange();
  }
*/
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

/*
Function for setting a lockedOut bool value and checking if the lockedOut value has changed
    - this then calls the reset function once lockedOut == true
*/
bool lockedOut(WeaponMode* currentMode){
    long now_millis= millis();
    static bool lastLockedOutState = false;
    return now_millis - timeHitWasMade >= currentMode->lockoutTime; // Return true if elapsed time is greater or equal to lockout time
}

// Reset all variables, needs to be right above setup() to have access to all the variables declared here
// time until LEDs are turned is implimented here. 
void resetValues(){
    int32_t now_micros = micros();
    unsigned long currentMillis = millis(); // get the current time
    if (currentMillis - lastResetTime >= /*LIGHTTIME*/ 2000) { // check if delayTime has passed
        // wait before turning off the buzzer
        digitalWrite(BUZZER_PIN,  LOW);
        //Removed this from all Fill color commands and placed here to see if this command is being excicuted
        fill_solid(pixelsl, NUMPIXELS, CRGB::Black);
        fill_solid(pixelsl, NUMPIXELS, CRGB::Black);
        FastLED.show();
        // reset the flags
        OnTargetA_Flag = false;
        OnTargetB_Flag = false;
        OffTargetA_Flag = false;
        OffTargetB_Flag = false;
        // set the new lastResetTime
        
        lastResetTime = currentMillis; // save the last time you reset the values
        timeHitWasMade = now_micros; // reset the timeHitWasMade
    }
}

//Above the setup function is where we will initialize the ADS1115 and the FastLED library, as well as declare our global variables.
//This is also where we will set the interrupt pin and attach the interrupt to the button ISR.
void setup() {
    Serial.begin(BAUDRATE);
    Serial.println("Hello!");
    Serial.println("Getting single-ended readings from AIN0..3");
    Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV)");
    /*ESP32 supports both I2C Standard-mode (Sm) and Fast-mode (Fm) which can go up to 100KHz and 400KHz respectively*/
    Wire.begin(21, 22); //sda, scl
    Wire.setClock(400000); //Increase I2C clock speed to 400kHz
    // ads1115_weaponONTARGET_A_B.begin(0x49);
    // ads1115_weaponOFFTARGET_A_B.begin(0x48);
    while (!Serial); // Wait for the serial port to connect
    if (!ads1115_weaponONTARGET_A_B.begin(0x48)) {
        Serial.println("Failed to initialize ads1115_weaponONTARGET_A_B at default 0x48!");
    }
    if (!ads1115_weaponOFFTARGET_A_B.begin(0x49)) {
        Serial.println("Failed to initialize ads1115_weaponOFFTARGET_A_B at address 0x49!");
    }
    Serial.println("Getting single-ended readings from AIN0..3");
    Serial.println("ADC GAIN_ONE Range: +/- 4.096V (1 bit = 0.125mV/ADS1115)");
    //====
    ads1115_weaponONTARGET_A_B.setGain(GAIN_ONE);
    ads1115_weaponOFFTARGET_A_B.setGain(GAIN_ONE);

    uint32_t reading = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0);
    if (reading == 0xFFFF) {
        Serial.println("Error reading from ads1115_weaponONTARGET_A_B!");
        // Handle the error
    }
    /*
    If weapon value is greater than 13100 then it is on target

    If weapon value is greater than 26000 then it is off target
    ADS1 Checks for onTarget PIN; 0 = WeaponA, 1 = LameA, 2 = WeaponB, 3 = LameB
    ADS2 Checks for OffTarget PIN; 0 = WeaponA, 1 = LameA, 2 = WeaponB, 3 = LameB

    Setup 3V comparator on channel 1,3 for Lame A and B for the OnTarget checks
    Setup 3V comparator on channel 0,2 for Weapon A and B for the OffTarget checks
    void startComparator_SingleEnded(uint8_t channel, int16_t threshold);
    this will trigger the interrupt pin when the value is greater than the threshold on the ALRT pin
    */
    
    //=============ADS_with On target threshold=======
    uint16_t weaponA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(3); // LameA
    //=============ADS_with Off target threshold=======
    uint16_t weaponA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(3); // LameA
    
    ads1115_weaponONTARGET_A_B.startComparator_SingleEnded(1, 13100);//WeaponA would hitOnTargetA = true pin 0
    ads1115_weaponONTARGET_A_B.startComparator_SingleEnded(3, 13100);//WeaponB would hitOnTargetB = true pin 3
    ads1115_weaponOFFTARGET_A_B.startComparator_SingleEnded(0, 26100);//WeaponA would hitOffTargetA = true pin 0
    ads1115_weaponOFFTARGET_A_B.startComparator_SingleEnded(2, 26100);//WeaponB would hitOffTargetB = true pin 3
    //Set ALRT PIN to an interrupt so that it can be used as a flag later
    pinMode(WeaponA_ONTARGET_ALRT_PIN, INPUT);
    pinMode(WeaponB_ONTARGET_ALRT_PIN, INPUT);
    pinMode(WeaponA_OFFTARGET_ALRT_PIN, INPUT);
    pinMode(WeaponB_OFFTARGET_ALRT_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(WeaponA_ONTARGET_ALRT_PIN), ISR_OnTargetA, RISING);
    attachInterrupt(digitalPinToInterrupt(WeaponB_ONTARGET_ALRT_PIN), ISR_OnTargetB, RISING);
    attachInterrupt(digitalPinToInterrupt(WeaponA_OFFTARGET_ALRT_PIN), ISR_OffTargetA, RISING);
    attachInterrupt(digitalPinToInterrupt(WeaponB_OFFTARGET_ALRT_PIN), ISR_OffTargetB, RISING);

    //FAST LED SETUP
    FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(pixelsl, NUMPIXELS);
    FastLED.addLeds<CHIPSET, DATA_PIN2, COLOR_ORDER>(pixelsr, NUMPIXELS);
    FastLED.setBrightness(BRIGHTNESS);
    //Mode button ISR
    pinMode(MODE_BUTTON_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(MODE_BUTTON_PIN), BUTTON_ISR, FALLING);
    resetValues();
}



//the main Loop where the program runs
void loop(){
        //=============ADS_with On target threshold=======
    uint16_t weaponA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OnTarget = ads1115_weaponONTARGET_A_B.readADC_SingleEnded(3); // LameA
    //=============ADS_with Off target threshold=======
    uint16_t weaponA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(0); // WeaponA
    uint16_t lameB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(1); // LameB
    uint16_t weaponB_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(2); // WeaponB
    uint16_t lameA_ADS_OffTarget = ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(3); // LameA
    Serial.println("Foil Mode");
    //     String serialData =   String("OntargetA : ") + weaponA_ADS_OnTarget + " : "+ lameB_ADS_OnTarget + "\n"
    //                 + String("OntargetB : ") + weaponB_ADS_OnTarget + " : "+ lameA_ADS_OnTarget +"\n"
    //                 + String("OfftargetA : ") + weaponA_ADS_OffTarget + " : "+ lameB_ADS_OnTarget +"\n"
    //                 + String("OfftargetB : ") + weaponB_ADS_OffTarget + " : "+ lameA_ADS_OnTarget +"\n";
    // Serial.println(serialData);
    //if the button is pressed then call modeChange()
    if(BUTTON_Debounce(MODE_BUTTON_PIN)){
    modeChange();
    }
    //this will be Weapon Mode Independant
    //make the HitTypeINT = 1
    // Use a switch-case statement to handle each flag
    switch (flagState) {
        case OnTargetA_Flag_Value:
            OnTargetA_Flag = false;
            Ident_OnTargetA(&OnTargetA);
            break;
        case OnTargetB_Flag_Value:
            OnTargetB_Flag = false;
            Ident_OnTargetB(&OnTargetB);
            break;
        case OffTargetA_Flag_Value:
            OffTargetA_Flag = false;
            Ident_OffTargetA(&OffTargetA);
            break;
        case OffTargetB_Flag_Value:
            OffTargetB_Flag = false;
            Ident_OffTargetB(&OffTargetB);
            break;
        default:
            break;
    }

    switch(currentMode->Mode_INT){
        
        case 1: //Foil

            //check lockout timer
            //if lockOut is true then -> Buzzer ON and Stop Clock
            //Trun Buzzer ON *these should only happen once the lockout has happened
            //Stop the clock *these should only happen once the lockout has happened
            //if locked out is true then call resetValues();
            //check the lock out timer, if(true){resetValues();, break;}
            if(lockedOut(currentMode)){
                resetValues();
                digitalWrite(BUZZER_PIN, HIGH); //Buzzer ON
                //Stop Clock
            }
            switch(HitTypes){
                case 6: //OnTargetA
                        //Set lockout timer
                        //Show LEDs
                        Serial.print("onTargetA:"); Serial.print(HitTypes); Serial.print(": "); Serial.print(weaponA_ADS_OnTarget); Serial.print(": "); Serial.println(lameB_ADS_OnTarget);
                        // Serial.print(weaponA_ADS_OnTarget); Serial.println(lameB_ADS_OnTarget);
                        timeHitWasMade = micros();
                        fill_solid(pixelsl, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 7: //OnTargetB
                        Serial.println("onTargetB"); Serial.print(HitTypes); Serial.print(": "); Serial.print(weaponB_ADS_OnTarget); Serial.print(": "); Serial.println(lameA_ADS_OnTarget);
                        // Serial.print(weaponB_ADS_OnTarget); Serial.println(lameA_ADS_OnTarget);
                        timeHitWasMade = micros();
                        fill_solid(pixelsr, NUMPIXELS, CRGB::Red); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 8: //OffTargetA
                        Serial.println("offTargetA"); Serial.print(HitTypes); Serial.print(": "); Serial.print(weaponA_ADS_OffTarget); Serial.print(": "); Serial.println(lameB_ADS_OffTarget);
                        timeHitWasMade = micros();
                        fill_solid(pixelsl, NUMPIXELS, CRGB::Yellow); // Moderately bright RED color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 9: //OffTargetB
                        Serial.println("offTargetB"); Serial.print(HitTypes); Serial.print(": "); Serial.print(weaponB_ADS_OffTarget); Serial.print(": "); Serial.println(lameA_ADS_OffTarget);
                        timeHitWasMade = micros();
                        fill_solid(pixelsr, NUMPIXELS, CRGB::Blue); // Moderately bright RED color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 0: //0 then we do nothing no hit has been made
                break;
            break; //End of Foil Case
            }
        case 2: //Epee
            switch(HitType -> HitTypeINT){
                case 6: //OnTargetA
                        Serial.println("onTargetB");
                        timeHitWasMade = micros();
                        fill_solid(pixelsl, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 7: //OnTargetB
                        Serial.println("onTargetB");
                        timeHitWasMade = micros();
                        fill_solid(pixelsr, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                default: //Should never reach this point unless there's an error in the code or something
                break;
        break; //End of Epee Case
            }
        case 3: //Sabre
            switch(HitType -> HitTypeINT){
                case 6: //OnTargetA
                        Serial.println("onTargetB");
                        timeHitWasMade = micros();
                        fill_solid(pixelsr, NUMPIXELS, CRGB::Green); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 7: //OnTargetB
                        Serial.println("onTargetB");
                        timeHitWasMade = micros();
                        fill_solid(pixelsr, NUMPIXELS, CRGB::Red); // Moderately bright GREEN color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                case 8: //OffTargetA
                        Serial.println("offTargetA");
                        timeHitWasMade = micros();
                        fill_solid(pixelsl, NUMPIXELS, CRGB::Yellow); // Moderately bright RED color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);

                break;
                case 9: //OffTargetB
                        Serial.println("offTargetB");
                        timeHitWasMade = micros();
                        fill_solid(pixelsl, NUMPIXELS, CRGB::Blue); // Moderately bright RED color.
                        FastLED.show();
                        digitalWrite(BUZZER_PIN, HIGH);
                break;
                default: //Should never reach this point unless there's an error in the code or something
                break;
        break; //End of Sabre Case
            }
        default:
        //fail safe to make sure we catch a null case or some other error
            break;
    }



} //end of void Loop

