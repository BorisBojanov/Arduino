//Score_Box_V5_with_ALRT_PIN
#include <FastLED.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ADS1X15.h>


// Enumeration for modes
typedef enum {
 FOIL,
 EPEE,
 SABRE
} WeaponMode;
WeaponMode currentMode;
//Global Variables
 

//if variables need to be used by ISR mkae them Volatile
//if  variables change often and need to be loaded quickly make them Volatile
bool lockedOut    = false;
bool depressedA   = false;
bool depressedB   = false;
bool hitOnTargA  = false;
bool hitOffTargA = false;
bool hitOnTargB  = false;
bool hitOffTargB = false;
long depressAtime = 0;
long depressBtime = 0;

//small functions
void Something_ISR_1() {}
void Something_ISR_2() {}
void Something_ISR_3() {}
void modeChangeISR() {}

//logic for the three modes
void foil(){
    const int foilMode[] = {300000, 14000};
}
void epee(){
    const int epeeMode[] = {45000, 2000};
}
void sabre(){
    const int sabreMode[] = {170000, 1000};
}

// Function to handle the selected mode
void handleMode() {
 switch (currentMode) {
    case FOIL:
      foil();
      break;
    case EPEE:
      epee();
      break;
    case SABRE:
      sabre();
      break;
    default:
      // Error handling if an invalid mode is selected
      break;
 }
}

resetValues(){
    lockedOut    = false;
    depressAtime = 0;
    depressedA   = false;
    depressBtime = 0;
    depressedB   = false;
    hitOnTargA  = false;
    hitOffTargA = false;
    hitOnTargB  = false;
    hitOffTargB = false;

    delay(5);
}
//main functions
void setup() {

 // Set an initial mode, for example, FOIL
 currentMode = FOIL;
}

void loop() {
//This loop should call one function that is a pointer to the mode that has been selected
//This function should be called in a switch statement?
//A function that checks the if any interrupts have set a flag to Ture

 // Check if any interrupts have set a flag to True
 if (/* check for interrupt flags */) {
    modeChangeISR();
 }

 // Call the function for the current mode
 handleMode();

}

