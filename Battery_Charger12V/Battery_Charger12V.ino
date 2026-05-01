/*
  Battery charger, chargest 12V batteries. 
  Battery needs to be chgarged with 15V input. 
  Battery will be charged once the voltage between the contacts is 14V
  Bulk Charge (Initial Stage): 14.4V to 14.7V is optimal for bringing a 12V battery up to full charge.
  Float Charge (Maintenance): 13.5V to 13.8V is used to keep a battery charged without damaging it.

  Temperature Impact: Higher temperatures (> 25C) require lower voltages, while colder temperatures require higher voltages to charge effectively.
  Maximum Limit: Avoid exceeding 14.7V, as this can lead to electrolyte loss and damage the battery. 
  
  Fully Charged (Resting) 12.7V - 12.8V
  Automatically adjusts between bulk and float voltages

  IRF4905 (P-channel, 55 V) MOSFET
  JQC-3FF-S-Z 12 V relay
  Zener Diode

  Need:
    A buck/linear control loop to regulate charge current/voltage

    If your P-MOSFET source is sitting at ~12–15 V, then the gate must be driven relative to that source.
      Source → charger/buck output (the “supply” feeding the battery)
      Drain → battery +
      Gate: 
        Pull-up resistor from gate to source = default OFF
        A small transistor pulls gate down to turn it ON
      
    gate driver using a small NPN transistor
    2N222 / BC547
      Gate resistor (series)
      Gate pull-up resistor
      Optional gate-source zener clamp
    Rpull from Gate → Source
    NPN collector to Gate (through a small Rgate series resistor)
    NPN emitter to GND
    ESP32 GPIO → Rbase → NPN base

    Operation:
    ESP32 LOW → NPN off → gate pulled up to source → MOSFET OFF
    ESP32 HIGH → NPN on → gate pulled down → MOSFET ON

  Protect VGS:
    If your charger output can be, say, 15–18 V, pulling gate to ground makes VGS = −(15–18) V, which is within ±20 V, 
    but close enough that spikes can hurt you. 
    A ~15 V zener from Gate to Source (cathode at source, anode at gate) is common protection.

  Calculate the resistor values:
    Gate pull-up resistor (Rpull)
      keep MOSFET OFF by default, and not waste power.
      Typical range: 47 kΩ to 200 kΩ.
      choose it based on how “stiff” you want the OFF state and how noise-sensitive your layout is:
        Noisy/high-current wiring → prefer 47–100 kΩ
        Cleaner layout → 100–200 kΩ is fine
    Gate series resistor (Rgate)
      limit ringing, reduce EMI, and control switching edge speed.
      Typical: 47 Ω to 220 Ω.
      If you’ll do only on/off (not PWM), anything in that range is fine.
    Base resistor (Rbase) for the NPN
    You want enough base current to pull the MOSFET gate down firmly.
    Rule for a saturated BJT switch:
      Ib = Ic/10
      Ic is the gate discharge current while switching
      Ig = Qg/t_{sw}
      usage Qg ~ 120nC
      for t_{sw} = 1ms
      Ig = 120nC/1ms = 0.12mA
       Even if it switch 100× faster (10 µs), it’s only 12 mA briefly.
      Picked Ib 1-3mA
      Ib = 2mA
      Rbase = (V_gpio - 0.7) / Ib
      Rbase = (3.3V -0.7)/ 0.002 = 1300 Ω
      So 1.2 kΩ to 2.2 kΩ

  Depending on how you orient the 2N222 / BC547
    battery back-feeding into your supply
    supply “sneak paths” when you think it’s off

    Reverse-current blocking, common solutions are:
      Back-to-back MOSFETs (ideal-diode style), or
      A relay in series
      A proper ideal diode / power-path controller IC

  JQC-3FF-S-Z relay
  relay’s 12 V coil data is commonly around 400 Ω and ~30 mA coil current
  !!Do not drive the coil directly from ESP32!!
    NPN or N-MOSFET low-side driver
    Flyback diode across the coil
  Coil current estimate:
    I = V/R 12/400 = 0.03A = 30mA
    That is beyond waht should be pulled from the GPIO

  Power-stage considerations
    Smart charging needs robust analog/power
    Charger needs current regulation:
      A buck converter set up for CC/CV
      A dedicated charger controller IC/module suitable for your battery chemistry

    Required protections
      Fuse close to the battery
      Reverse polarity protection
      Over-voltage (hardware cutoff, not just software)
      Over-current / short circuit protection
      Thermal monitoring (MOSFET, inductor, sense resistor)
      Layout for high current (thick traces, short loops)

    Measuring voltage/current with ESP32
      Battery voltage must be scaled to ≤3.3 V with a divider. ESP32 pins are not 5 V tolerant.
      Divider formula:
        V_adc = V_bat (R2)/(R1 + R2)
      Currect Sensing:
        a low-side shunt resistor + amplifier, Or
        a high-side current sense IC (cleaner approach)
  Overall Archetecture
    Use a known CC/CV buck module or charger controller matched to your battery chemistry
    Use the IRF4905 as a high-side “charge enable” switch (with the transistor gate driver above)
    Use the relay as a hard safety disconnect (fault latch-off)
    ESP32 job:
      reads voltage/current/temperature
      decides state machine (bulk/absorb/float or CC/CV)
      never relies on software alone for emergency limits
*/

// #include "Wire.h"
// int val = 0, valLSB = 0 , valMSB = 0;
// byte addr = 0x75, mode = 0x6, regLSB = 0x0, regMSB = 0x1; // I2C address DCDC converter and gerister address



// int D2 = 2;

// void setup() {
//   // put your setup code here, to run once:
//   Wire.begin();
//   Wire.beginTransmission(addr);
//   Wire.write(mode);
//   Wire.write(B10100000); //Turn on the DCDC converter
//   Wire.endTransmission();
  
//   Serial.begin(115200);
// }

// void loop() {
//   // put your main code here, to run repeatedly:

//   // If there is some content on the serial interface
//   if(Serial.available() >  0){
//     val = Serial.readString().toInt();

//     val = constrain(val, 0, 2047); // only accept within range
//     valLSB = val & 255; // 255 = B00011111111 mask to fit only 1 byte (8 bit)
//     valMSB = (val & 1792) >> 8; //1792 = B11100000000 mask to fit only 3 bits and then byte shifted to erase other bits

//     Wire.beginTransmission(addr);
//     Wire.write(regLSB); // Write on LSB register
//     Wire.write(valLSB); // Write LSB byte
//     Wire.endTransmission();
//     delay(5);

//    Wire.beginTransmission(addr);


//   }
// }

// void turnOn (int addr) {
//   Wire.begin();
//   Wire.beginTransmission(addr);
//   Wire.write(0x6);
//   Wire.write(B10100000); //Turn on the DCDC converter
//   Wire.endTransmission();
// }

// void setVoltage(int addr, int val){
//     int regLSB = 0x0;
//     int valLSB = 0;
//     val = constrain(val, 0, 2047); // only accept within range
//     valLSB = val & 255; // 255 = B00011111111 mask to fit only 1 byte (8 bit)
//     valMSB = (val & 1792) >> 8; //1792 = B11100000000 mask to fit only 3 bits and then byte shifted to erase other bits

//     Wire.beginTransmission(addr);
//     Wire.write(regLSB); // Write on LSB register
//     Wire.write(valLSB); // Write LSB byte
//     Wire.endTransmission();
//     delay(5);

//    Wire.beginTransmission(addr);
// }

// ---------- Pins ----------
static const int PIN_CHARGE_EN = 25;      // GPIO to enable charging (HIGH = ON)
static const int PIN_ADC_BAT   = 34;      // ADC1 pin for battery sense (input only)
static const int PIN_ADC_NTC   = 35;      // Optional: NTC ADC (input only). If unused, set USE_NTC=false

// ---------- User config ----------
static const bool USE_NTC = false;        // set true if you wire an NTC divider

// Divider values for battery voltage sensing
// Example from earlier: R1=100k (top), R2=22k (bottom)
static const float R1_OHMS = 1000.0f;
static const float R2_OHMS = 10000.0f;

// ADC reference / scaling
// ESP32 ADC is 0-4095 counts at 12-bit, but effective full-scale depends on attenuation.
// We'll use attenuation setting + a nominal Vref and then rely on calibration factor.
static const int   ADC_BITS = 12;
static const int   ADC_MAX  = (1 << ADC_BITS) - 1;

// Nominal ADC full-scale voltage with ADC_11db attenuation is ~3.3V (varies!)
static const float ADC_FS_VOLTS_NOMINAL = 3.3f;

// Calibration factor (tune this with a multimeter)
// If your reported battery voltage is low/high, adjust this multiplier.
static const float VBAT_CAL = 1.00f;

// Charger voltage targets @ 25C (typical AGM ranges you listed)
static const float V_ABSORB_25C = 14.6f;  // within 14.4–14.7
static const float V_FLOAT_25C  = 13.7f;  // within 13.5–13.8

// Temperature compensation (simple)
// Many lead-acid chargers use about -3 mV/°C/cell (~ -18 mV/C for 12V battery, 6 cells)
// We'll implement: V_target = V_25C + slope*(T - 25C)
static const float TEMP_SLOPE_V_PER_C = -0.018f; // volts per C for 12V (6 cells)

// Safety limits
static const float V_OVERVOLT = 14.8f;    // hard software cutoff (below “don’t exceed 14.7” + noise margin)
static const float V_REVERSE  = -0.5f;    // sanity check (won't happen with divider properly referenced)
static const float V_MIN_SANE = 1.0f;     // below this -> sensor/wiring issue if a 12V battery is attached
static const float V_MAX_SANE = 16.5f;    // above this -> sensor/wiring issue

// Timing
static const unsigned long SAMPLE_MS      = 250;          // ADC sample rate
static const unsigned long ABSORB_MAX_MS  = 2UL * 60UL * 60UL * 1000UL; // 2 hours max absorb (tunable)
static const unsigned long IDLE_RETRY_MS  = 10UL * 1000UL;              // retry interval in idle

// Decision thresholds
static const float V_FULL_REST = 12.7f;   // resting full, used for simple idle decisions
static const float V_START_MIN = 10.5f;   // below this battery is deeply discharged; still can charge but watch heating

void setup(){

}


void loop(){

}

