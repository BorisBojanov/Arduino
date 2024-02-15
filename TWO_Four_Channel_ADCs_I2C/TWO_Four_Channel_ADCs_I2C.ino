#include <Wire.h>
#include <C:\Users\boris\OneDrive\Documents\Arduino\libraries\Adafruit_ADS1X15\Adafruit_ADS1X15.h>

#define ADS_A 0x48 // ADDR Connected to GND
#define ADS_B 0x49 // ADDR Connected to VCC
#define ADS_C 0x4A // ADDR Connected to SDA
#define ADS_D 0x4B // ADDR Connected to SCL

// Assign Variables
uint16_t RawValue_ADS_A; // Raw value from the ADC 16bit
uint16_t RawValue_ADS_B; // Raw value from the ADC 16bit
uint16_t RawValue_ADS_C; // Raw value from the ADC 16bit
uint16_t RawValue_ADS_D; // Raw value from the ADC 16bit
uint16_t ConvertedValue; // Contains the End result of the ADS rawdata
byte error; // Contains the error code
int HiLimit = 2.4; //3.0V this allows us to contain decimals
int LoLimit = 1; //2.5V this allows us to contain decimals

int HiOffTargetLimit = 3.75; //3.0V this allows us to contain decimals
int LoOffTargetLimit = 2.5; //3.0V this allows us to contain decimals

//PIN connections
int WeaponA_ONTARGET_ALRT_PIN = 5; // ADS1115 Ready Pin
int WeaponB_ONTARGET_ALRT_PIN = 18; // ADS1115 Ready Pin
int WeaponA_OFFTARGET_ALRT_PIN = 4; // ADS1115 Ready Pin
int WeaponB_OFFTARGET_ALRT_PIN = 15; // ADS1115 Ready Pin


// ADS1115 Register Addresses, These are pointers to the address we want to access
const short ConvReg     = 0b00000000; // Conversion Register Stores the result of the last conversion
const short CfgReg      = 0b00000001; // Config Register contains settings for how the ADC operates
const short LoThreshReg = 0b00000010; // Low Threshold Register
const short HiThreshReg = 0b00000011; // High Threshold Register
//==============================MSB=======================================================
/* Select the Most Segnificant Byte, bits 8-15
    Bits are reas from left to right after the "0b"
Bit 15    0=no effect, 1=Begin Single Conversion (in power down mode)
Bit 14    Configure pins for (0) comparitor mode or (1) single end mode
Bit 13:12 if Bit 14 is 0 then 00=A0 compared to A1. 01=A0 >A3, 10=A1>A3, 11=A2>A3. A0>A1 || A0>A3 || A1>A3 || A2>A3
          if Bit 14 is 1 then 00=A0 > GND, 01=A1 > GND, 10=A2 > GND, 11=A3 > GND
Bit 11:9  Programable Gain 000=6.144v, 001=4.096v, 010=2.048v, 011=1.024v, 100=0.512v, 101=0.256v
bit 8     0=Continuous Conversion, 1=Power Down Single Shot
*/
                    //bits:15,14,13:12,11:9,8
  const short MSBcfg  = 0b01000010; // config for single-ended Continous conversion for 4.096v Readings on A0               
//const short MSBcfg  = 0b01000010; // config for single-ended Continous conversion for 4.096v Readings on A0
//const short MSBcfg2 = 0b01010010; // config for single-ended Continous conversion for 4.096v Readings on A1
//const short MSBcfg3 = 0b01100010; // config for single-ended Continous conversion for 4.096v Readings on A2
//const short MSBcfg4 = 0b01110010; // config for single-ended Continous conversion for 4.096v Readings on A3

const short MSBcfgCOMPAR_A0_A1 = 0b00000010; // config for comparitor A0 >A1 for 4.096v Readings, Continuous Conversion
const short MSBcfgCOMPAR_A0_A3 = 0b00010010; // config for comparitor A0 >A3 for 4.096v Readings, Continuous Conversion
// const short MSBcfgCOMPAR_A1_A3 = 0b00100010; // config for comparitor A1 >A3 for 4.096v Readings, Continuous Conversion
//const short MSBcfgCOMPAR_A2_A3 = 0b00110010; // config for comparitor A2 >A3 for 4.096v Readings, Continuous Conversion
//=================================LSB===================================================
/* Least Segnificant Byte, bits 0-7
    Bits are reas from left to right after the "0b"
Bits 7:5  000=8sps, 001=16sps, 010=32sps, 011=64sps, 100=128sps, 101=250sps, 110=475sps, 111=860/sps
Bit  4    0=Traditional Comparator with hysteresis, 1=Window Comparator
Bit  3    Alert/Ready Pin 0= Goes Low when it enters active state or 1= goes High when it enters active state
Bit  2    Alert/Ready pin Latching 0=No (pulse the Pin after each conversion), 1=Yes (stays in current state until the data is read)
Bits 1:0  Number of conversions to complete before PIN is active, 00=1, 01=2, 10=4, 11=disable comparator and set to default
*/                       //bits:7:5,4,3,2,1:0
                                    // 100       0                 0                          0                                  10
//const short LSBcfg  = 0b00001010; // 128sps, Traditional Mode, High at the end of the conversion, pulse the Pin after each conversion, 4 conversions          
                      //bits:7:5,4,3,2,1:0
                                   // 011       0                  0                          0                                  00
//const short LSBcfg = 0b01100010; // 64x sps, Traditional Mode, Low when in active state, pulse the Pin after each conversion, 1 conversions
                                 // 000       1             0                           0                                 10
  const short LSBcfg = 0b11110010;// 8x sps, Window mode, Low when in active state, pulse the Pin after each conversion, 4 conversions
                                 // goes ACTIVE after the fourth conversion, so we do 4 confirmations.

volatile int16_t timeHitWasMade; // This is the time the hit was made
volatile bool ISR_StateChanged = false; // This is the flag that is set when the button is pressed
volatile bool OnTargetA_Flag   = false;
volatile bool OnTargetB_Flag   = false;
volatile bool OffTargetA_Flag  = false;
volatile bool OffTargetB_Flag  = false;

// Assign each flag a unique power of 2
const int OnTargetA_Flag_Value = 2; // 2^1
const int OnTargetB_Flag_Value = 4; // 2^0
const int OffTargetA_Flag_Value = 8; // 2^3
const int OffTargetB_Flag_Value = 16; // 2^4

// Calculate the state of the flags
int flagState = (OnTargetA_Flag * OnTargetA_Flag_Value) +
                (OnTargetB_Flag * OnTargetB_Flag_Value) +
                (OffTargetA_Flag * OffTargetA_Flag_Value) +
                (OffTargetB_Flag * OffTargetB_Flag_Value);

void ISR_OnTargetA(){//This function is potentially being called very often 
    timeHitWasMade = micros();
    OnTargetA_Flag = true;

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

void setup(void) {
  pinMode(WeaponA_ONTARGET_ALRT_PIN, INPUT); // Set the RDY pin as an input
  pinMode(WeaponB_ONTARGET_ALRT_PIN, INPUT); // Set the RDY pin as an input
  pinMode(WeaponA_OFFTARGET_ALRT_PIN, INPUT); // Set the RDY pin as an input
  pinMode(WeaponB_OFFTARGET_ALRT_PIN, INPUT); // Set the RDY pin as an input
  Serial.begin(921600);
  Serial.println("Hello world");
  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 0.1875mV/ADS1115)"); 
  int HI = 16000; // 2V Convert the voltage to a 16bit value
  int LO = 8000;  // 1V Convert the voltage to a 16bit value
  int HIOffTarget = 30000; // 3.75V Convert the voltage to a 16bit value
  int LOOffTarget = 20000; // 2.5V Convert the voltage to a 16bit value
  attachInterrupt(digitalPinToInterrupt(WeaponA_ONTARGET_ALRT_PIN), ISR_OnTargetA, RISING);
  attachInterrupt(digitalPinToInterrupt(WeaponB_ONTARGET_ALRT_PIN), ISR_OnTargetB, RISING);
  attachInterrupt(digitalPinToInterrupt(WeaponA_OFFTARGET_ALRT_PIN), ISR_OffTargetA, RISING);
  attachInterrupt(digitalPinToInterrupt(WeaponB_OFFTARGET_ALRT_PIN), ISR_OffTargetB, RISING);
  Wire.begin();
  Wire.setClock(400000); //Increase I2C clock speed to 400kHz
//======================ADS_A================================
  delay(10);
  //the following will disable the HI/LO Thresholds registers to use pin ## as the READY pin
  Wire.beginTransmission(ADS_A); // Start I2C Transmission to the ADS1115
  Wire.write(HiThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(HI)); //Write the MSB of the HI Threshold
  Wire.write(lowByte(HI)); //Write the LSB of the HI Threshold
  // Wire.write(0b10000000); // the 1 states that this is the MSB Set the HI Threshold to 0
  // Wire.write(0b00000000); // the 0 states that this is the LSB Set the HI Threshold to 0
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10); //wait 10milliseconds before sending the next I2C transmission

  Wire.beginTransmission(ADS_A); // Start I2C Transmission to the ADS1115
  Wire.write(LoThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(LO)); //Write the MSB of the LO Threshold
  Wire.write(lowByte(LO)); //Write the LSB of the LO Threshold
  // Wire.write(0b01111111); // the 1 states that this is the MSB 
  // Wire.write(0b11111111); // the 0 states that this is the LSB 
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10);
  //This is sending the configuration to the ADS1115
  Wire.beginTransmission(ADS_A); // Start I2C Transmission to the ADS1115
  Wire.write(CfgReg); // write the register address to point to the configuration register
  Wire.write(MSBcfgCOMPAR_A0_A1); // Write the MSB of the configuration register
  Wire.write(LSBcfg); // Write the LSB of the configuration register
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Configuration Set for ADS_A");
  }
//======================ADS_B================================
  delay(10);
  //the following will disable the HI/LO Thresholds registers to use pin ## as the READY pin
  Wire.beginTransmission(ADS_B); // Start I2C Transmission to the ADS1115
  Wire.write(HiThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(HI)); //Write the MSB of the HI Threshold
  Wire.write(lowByte(HI)); //Write the LSB of the HI Threshold
  // Wire.write(0b10000000); // the 1 states that this is the MSB Set the HI Threshold to 0
  // Wire.write(0b00000000); // the 0 states that this is the LSB Set the HI Threshold to 0
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10); //wait 10milliseconds before sending the next I2C transmission

  Wire.beginTransmission(ADS_B); // Start I2C Transmission to the ADS1115
  Wire.write(LoThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(LO)); //Write the MSB of the LO Threshold
  Wire.write(lowByte(LO)); //Write the LSB of the LO Threshold
  // Wire.write(0b01111111); // the 1 states that this is the MSB 
  // Wire.write(0b11111111); // the 0 states that this is the LSB 
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10);
  //This is sending the configuration to the ADS1115
  Wire.beginTransmission(ADS_B); // Start I2C Transmission to the ADS1115
  Wire.write(CfgReg); // write the register address to point to the configuration register
  Wire.write(MSBcfgCOMPAR_A0_A1); // Write the MSB of the configuration register
  Wire.write(LSBcfg); // Write the LSB of the configuration register
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Configuration Set for ADS_B");
  }
//======================ADS_C================================
  delay(10);
  //the following will disable the HI/LO Thresholds registers to use pin ## as the READY pin
  Wire.beginTransmission(ADS_C); // Start I2C Transmission to the ADS1115
  Wire.write(HiThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(HIOffTarget)); //Write the MSB of the HI Threshold
  Wire.write(lowByte(HIOffTarget)); //Write the LSB of the HI Threshold
  // Wire.write(0b10000000); // the 1 states that this is the MSB Set the HI Threshold to 0
  // Wire.write(0b00000000); // the 0 states that this is the LSB Set the HI Threshold to 0
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10); //wait 10milliseconds before sending the next I2C transmission

  Wire.beginTransmission(ADS_C); // Start I2C Transmission to the ADS1115
  Wire.write(LoThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(LOOffTarget)); //Write the MSB of the LO Threshold
  Wire.write(lowByte(LOOffTarget)); //Write the LSB of the LO Threshold
  // Wire.write(0b01111111); // the 1 states that this is the MSB 
  // Wire.write(0b11111111); // the 0 states that this is the LSB 
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10);
  //This is sending the configuration to the ADS1115
  Wire.beginTransmission(ADS_C); // Start I2C Transmission to the ADS1115
  Wire.write(CfgReg); // write the register address to point to the configuration register
  Wire.write(MSBcfg); // Write the MSB of the configuration register
  Wire.write(LSBcfg); // Write the LSB of the configuration register
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Configuration Set for ADS_C");
  }
//======================ADS_D================================
  delay(10);
  //the following will disable the HI/LO Thresholds registers to use pin ## as the READY pin
  Wire.beginTransmission(ADS_D); // Start I2C Transmission to the ADS1115
  Wire.write(HiThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(HIOffTarget)); //Write the MSB of the HI Threshold
  Wire.write(lowByte(HIOffTarget)); //Write the LSB of the HI Threshold
  // Wire.write(0b10000000); // the 1 states that this is the MSB Set the HI Threshold to 0
  // Wire.write(0b00000000); // the 0 states that this is the LSB Set the HI Threshold to 0
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10); //wait 10milliseconds before sending the next I2C transmission

  Wire.beginTransmission(ADS_D); // Start I2C Transmission to the ADS1115
  Wire.write(LoThreshReg); // 16bit register, so we must send the number in two parts
  Wire.write(highByte(LOOffTarget)); //Write the MSB of the LO Threshold
  Wire.write(lowByte(LOOffTarget)); //Write the LSB of the LO Threshold
  // Wire.write(0b01111111); // the 1 states that this is the MSB 
  // Wire.write(0b11111111); // the 0 states that this is the LSB 
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Command ackowledged");
  }

  delay(10);
  //This is sending the configuration to the ADS1115
  Wire.beginTransmission(ADS_D); // Start I2C Transmission to the ADS1115
  Wire.write(CfgReg); // write the register address to point to the configuration register
  Wire.write(MSBcfg); // Write the MSB of the configuration register
  Wire.write(LSBcfg); // Write the LSB of the configuration register
  error = Wire.endTransmission(); // End the I2C Transmission
  if (error > 0) {
    Serial.print("Error: ");
    Serial.println(error);
  } else {
    Serial.println("Configuration Set for ADS_D");
  }

}

void loop(void) {
  delay(20);
  int flagState = (OnTargetA_Flag * OnTargetA_Flag_Value) +
                  (OnTargetB_Flag * OnTargetB_Flag_Value) +
                  (OffTargetA_Flag * OffTargetA_Flag_Value) +
                  (OffTargetB_Flag * OffTargetB_Flag_Value);
  // "do" is a keyword known to C
  // do{
  //   //Serial.println("Doing Nothing, waiting for RDY pin to go high");
  //   //Do nothing until conversion is ready
  // } while(digitalRead(RDY) == 0); //Wait for the ADS1115 to be ready
  // if RDY pin is HIGH then the ADS1115 is ready to be read
  // Until then it will be stuck in this loop
  
  //Request Data
  Wire.beginTransmission(ADS_A); // Start I2C Transmission to the ADS1115
  Wire.write(ConvReg); //point to the convergen register to read conversion data
  Wire.endTransmission(); // End the I2C Transmission
  Wire.requestFrom(ADS_A, 2); // Request 2 bytes from the ADS1115 (MSB then the LSB)
  /* Since the ADS1115 sends the MSB first we need to Read the first byte (MSB) and shift it 8 bits to the left
  then we read the second byte (LSB) and add it to the first byte. This will give us the 16bit value of the conversion
  */
  RawValue_ADS_A = Wire.read() << 8 | Wire.read(); // Read the MSB and shift it 8 bits to the left, then read the LSB and add it to the MSB
  
  Wire.beginTransmission(ADS_B);
  Wire.write(ConvReg);
  Wire.endTransmission();
  Wire.requestFrom(ADS_B, 2); //Request the LSB of the data from ADS_C
  RawValue_ADS_B = Wire.read() << 8 | Wire.read(); // Read the MSB and shift it 8 bits to the left, then read the LSB and add it to the MSB

  Wire.beginTransmission(ADS_C);
  Wire.write(ConvReg);
  Wire.endTransmission();
  Wire.requestFrom(ADS_C, 2); //Request the LSB of the data from ADS_C
  RawValue_ADS_C = Wire.read() << 8 | Wire.read(); // Read the MSB and shift it 8 bits to the left, then read the LSB and add it to the MSB
  // Display the results
  // Serial.print("RawValue_ADS_A: "); Serial.print(RawValue_ADS_A); Serial.print("\t"); Serial.print("Alert/Ready pin = "); Serial.println(digitalRead(WeaponA_ONTARGET_ALRT_PIN)); //Display the voltage measured on A0 on the serial monitor
  // Serial.print("RawValue_ADS_C: "); Serial.print(RawValue_ADS_C); Serial.print("\t"); Serial.print("Alert/Ready pin = "); Serial.println(digitalRead(WeaponA_OFFTARGET_ALRT_PIN)); //Display the voltage measured on A0 on the serial monitor
  // Serial.print("RawValue_ADS_B: "); Serial.print(RawValue_ADS_B); Serial.print("\t"); Serial.print("Alert/Ready pin = "); Serial.println(digitalRead(WeaponB_ONTARGET_ALRT_PIN)); //Display the voltage measured on A0 on the serial monitor
  // Serial.print("RawValue_ADS_D: "); Serial.print(RawValue_ADS_D); Serial.print("\t"); Serial.print("Alert/Ready pin = "); Serial.println(digitalRead(WeaponB_OFFTARGET_ALRT_PIN)); //Display the voltage measured on A0 on the serial monitor
  Serial.println(flagState);
switch (flagState) {
    case OnTargetA_Flag_Value:
        OnTargetA_Flag = false;
        Serial.println("OnTargetA_DETECTED!!!");
        break;
    case OnTargetB_Flag_Value:
        OnTargetB_Flag = false;
        Serial.println("OnTargetB_DETECTED!!!");
        break;
    case OffTargetA_Flag_Value:
        OffTargetA_Flag = false;
        Serial.println("OffTargetA_DETECTED!!!");
        break;
    case OffTargetB_Flag_Value:
        OffTargetB_Flag = false;
        Serial.println("OffTargetB_DETECTED!!!");
        break;
    default:
        break;
}

// Display the results
// Serial.print("Raw Data:"); Serial.print(RawValue); Serial.print(" x 0.0001865uV =");
// Serial.print(ConvertedValue, 3);/*3 decimal places*/ Serial.println("Volts"); //Display the voltage measured on A0

}






