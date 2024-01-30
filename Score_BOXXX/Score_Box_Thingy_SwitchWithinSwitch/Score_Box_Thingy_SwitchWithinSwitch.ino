//Scorebox thingy with switch case(Foil, Epee, Sabre) leading into switch case(OnTargetA, OffTargetA, OnTargetB, OffTargetB)
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <Arduino.h>
#include <FastLED.h>
#include <iostream>
#define NUMPIXELS 64 // number of neopixels in strip
#define DATA_PIN 12  // input pin Neopixel is attached to
#define DATA_PIN2 19 // input for the seconf Neopixel LED
#define CHIPSET WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 20
#define MODE_BUTTON_PIN 23
#define BUZZER_PIN 5
struct CRGB pixels[NUMPIXELS];
struct CRGB pixels222[NUMPIXELS];
int lastButtonState = LOW; // The last known state of the button
int buttonState;
volatile bool ISR_StateChanged =false;
volatile bool OnTargetA_Flag = false;
volatile bool OnTargetB_Flag = false;
volatile bool OffTargetA_Flag = false;
volatile bool OffTargetB_Flag = false;

//Variable declaration for the switch cases
int32_t timeHitWasMade; // This is the time the hit was made
int32_t timerStartA;
int32_t timerStartB;
int32_t now_micros= micros();
int32_t now_millis= millis();

Adafruit_ADS1115 ads1115_weaponONTARGET_A_B;  // ADS1115 with ADDR pin floating (default address 0x48)
Adafruit_ADS1115 ads1115_weaponOFFTARGET_A_B;     // ADS1115 with ADDR pin connected to 3.3V (address 0x49)

// Representing 300 microseconds
uint32_t Foil_LOCKOUT_TIME = 300000;
// Representing 45 microseconds
uint32_t Epee_LOCKOUT_TIME = 45000;
// Representing 170 microseconds
uint32_t Sabre_LOCKOUT_TIME = 170000;
// const int32_t Foil_LOCKOUT_TIME = 300000; // 300,000 microseconds = 300 milliseconds
// const int32_t Epee_LOCKOUT_TIME = 45000; // 45,000 microseconds = 45 milliseconds
// const int32_t Sabre_LOCKOUT_TIME = 170000; // 170,000 microseconds = 170 milliseconds
int32_t Foil_DEPRESS_TIME = 2000; // 2000 microseconds = 2 milliseconds
int32_t Epee_DEPRESS_TIME = 1000; // 1000 microseconds = 1 milliseconds
int32_t Sabre_DEPRESS_TIME = 0; // 10 microseconds = .01 milliseconds
// Global variable to store the current weapon mode

int32_t Foil_lockout = 300000;
int32_t Epee_lockout = 45000;
int32_t Sabre_lockout = 120000;

int32_t Foil_Drepess = 14000;
int32_t Epee_Depress = 0;
int32_t Sabre_Depress = 10;

struct WeaponMode {
    //Variable declaration for the switch cases
    int32_t timeHitWasMade; // This is the time the hit was made
    long lockoutTime;
    long depressTime;
    int Mode_INT; //used by the switch statement to select weaponMode
};

struct HitType {
    int OnTargetA;
    int OffTargetA;
    int OnTargetB;
    int OffTargetB;
};
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

//Build the structs     long            long         int
WeaponMode foilMode =  {Foil_lockout,  Foil_Drepess,  1};
WeaponMode epeeMode =  {Epee_lockout,  Epee_Depress,  2};
WeaponMode sabreMode = {Sabre_lockout, Sabre_Depress, 3};
WeaponMode* currentMode = &foilMode; // Default Mode and Initializing the cariable currentMode

// HitType OnTargetA  = {1, 0, 0, 0};
// HitType OffTargetA = {0, 2, 0, 0};
// HitType OnTargetB  = {0, 0, 3, 0};
// HitType OffTargetB = {0, 0, 0, 4};


//each one of these takes the value of the Weapon from that side and then compares it to the lame of the other(with exception of Epee)
//if the value is greater than the threshold then it is a hit
// ADC_OnTarget_readings OnTargetA = {long WeaponA, long LameB, int HitType OnTargetA }
ADC_OnTarget_readings OnTargetA  = {ads1115_weaponONTARGET_A_B.readADC_SingleEnded(0), ads1115_weaponONTARGET_A_B.readADC_SingleEnded(1), 1 };
ADC_OnTarget_readings OnTargetB  = {ads1115_weaponONTARGET_A_B.readADC_SingleEnded(2), ads1115_weaponONTARGET_A_B.readADC_SingleEnded(3), 2};
ADC_OnTarget_readings OffTargetA = {ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(0), ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(1), 3 };
ADC_OnTarget_readings OffTargetB = {ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(2), ads1115_weaponOFFTARGET_A_B.readADC_SingleEnded(3), 4};
ADC_OnTarget_readings* HitType;
int HitTypes = HitType -> HitTypeINT;

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

void Ident_OnTargetA (int32_t Time_Of_Hit){
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
    WeaponMode currentMode;
    int32_t timeLimit = 0;
    timeLimit = currentMode.depressTime;
    timeLimit = timeLimit + now_millis;
        //(depress time fo mdoe) < (now - time hit was made)
        // we need to set timeHitWasMade when this function is fisrt called.
    if (timeLimit < (Time_Of_Hit)){
        HitTypes = OnTargetA.HitTypeINT;
        OnTargetA_Flag = false;
        //once this if statement is called we need to reset timeHitWasMade.
    }
}
void Ident_OnTargetB (int32_t Time_Of_Hit){
    WeaponMode currentMode;
    int32_t timeLimit = 0;
    timeLimit = currentMode.depressTime;
    timeLimit = timeLimit + now_millis;
        //(depress time fo mdoe) < (now - time hit was made)
        // we need to set timeHitWasMade when this function is fisrt called.
    if (timeLimit < (Time_Of_Hit)){
        (HitType -> HitTypeINT) = OnTargetB.HitTypeINT;
        //once this if statement is called we need to reset timeHitWasMade.
    }
}
void Ident_OffTargetA(int32_t Time_Of_Hit){
WeaponMode currentMode;
int32_t timeLimit = 0;
timeLimit = currentMode.depressTime;
timeLimit = timeLimit + now_millis;
        //(depress time fo mdoe) < (now - time hit was made)
        // we need to set timeHitWasMade when this function is fisrt called.
    if (timeLimit < (Time_Of_Hit)){
        (HitType -> HitTypeINT) = OffTargetA.HitTypeINT;
        //once this if statement is called we need to reset timeHitWasMade.
    }
}
void Ident_OffTargetB(int32_t Time_Of_Hit){    
WeaponMode currentMode;
int32_t timeLimit = 0;
timeLimit = currentMode.depressTime;
timeLimit = timeLimit + now_millis;
        //(depress time fo mdoe) < (now - time hit was made)
        // we need to set timeHitWasMade when this function is fisrt called.
    if (timeLimit < (Time_Of_Hit)){
        (HitType -> HitTypeINT) = OffTargetB.HitTypeINT;
        //once this if statement is called we need to reset timeHitWasMade.
    }
}
void ISR_OnTargetA(){//This function is potentially being called very often 

    if(OnTargetA_Flag == false){
    timeHitWasMade = now_micros;
    }
    OnTargetA_Flag = true;
}
void ISR_OnTargetB(){
    if (OnTargetB_Flag == false){    
    timeHitWasMade = now_micros;
    }
    bool OnTargetB_Flag = true;
}
void ISR_OffTargetA(){
    if(OffTargetA_Flag == false){
    timeHitWasMade = now_micros;
    }
    bool OffTargetA_Flag = true;
}
void ISR_OffTargetB(){
    if(OffTargetB_Flag == false){
    timeHitWasMade = now_micros;
    }
    bool OffTargetB_Flag = true;
}


void loop(){
//this will be Weapon Mode Independant
//make the HitTypeINT = 1

// Use a switch-case statement to handle each flag
switch (flagState) {
    case OnTargetA_Flag_Value:
        OnTargetA_Flag = false;
        Ident_OnTargetA(timeHitWasMade);
        break;
    case OnTargetB_Flag_Value:
        OnTargetB_Flag = false;
        Ident_OnTargetB(timeHitWasMade);
        break;
    case OffTargetA_Flag_Value:
        OffTargetA_Flag = false;
        Ident_OffTargetA(timeHitWasMade);
        break;
    case OffTargetB_Flag_Value:
        OffTargetB_Flag = false;
        Ident_OffTargetB(timeHitWasMade);
        break;
}









switch(currentMode->Mode_INT){
    case 1: //Foil
        //check lockout timer
        //if lockOut is true then -> Buzzer ON and Stop Clock
        //Trun Buzzer ON *these should only happen once the lockout has happened
        //Stop the clock *these should only happen once the lockout has happened
        switch(HitTypes){
            case 1: //OnTargetA
                    //Set lockout timer
                    //Show LEDs
            break;
            case 2: //OnTargetB
            break;
            case 3: //OffTargetA
            break;
            case 4: //OffTargetB
            break;
        }

        break; //End of Foil Case
    case 2: //Epee
        switch(HitType -> HitTypeINT){
            case 1: //OnTargetA
            break;
            case 2: //OnTargetB
            break;
        break; //End of Epee Case
        }
    case 3: //Sabre
        switch(HitType -> HitTypeINT){
            case 1: //OnTargetA
            break;
            case 2: //OnTargetB
            break;
            case 3: //OffTargetA
            break;
            case 4: //OffTargetB
            break;
        break; //End of Sabre Case
        }
    default:
    //fail safe to make sure we catch a null case or some other error
        break;
}

//if locked out is true then call reset
//need to reset
//LEDs OFF
//Buzzer OFF
//reset the lockout timer
//reset the depress timer


} //end of void Loop

