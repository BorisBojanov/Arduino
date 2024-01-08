#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads1115_default;  // ADS1115 with ADDR pin floating (default address 0x48)
Adafruit_ADS1115 ads1115_addr;     // ADS1115 with ADDR pin connected to GND

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello world");

  ads1115_default.begin(0x48);    // Initialize the first ADS1115 at default address 0x48 (ADDR is a Floating Pin)
  // ads1115_addr.begin(0x49);   // Initialize the second ADS1115 at address 0x49 (ADDR connected to VDD aka power of ADC Pin)

  //====
  while (!Serial); // Wait for the serial port to connect

  if (!ads1115_default.begin(0x48)) {
    Serial.println("Failed to initialize ADS1115 at default address!");
  }
  // if (!ads1115_addr.begin(0x49)) {
  //   Serial.println("Failed to initialize ADS1115 at address 0x49!");
  // }
  //====

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

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
  ads1115_default.setGain(GAIN_ONE);
  ads1115_addr.setGain(GAIN_ONE);

}

void loop(void) {
  int16_t adc0, adc1, adc2, adc3;
  // int16_t adc4, adc5, adc6, adc7;

  float volts0, volts1, volts2, volts3;
  // float volts4, volts5, volts6, volts7;

  
  adc0 = ads1115_default.readADC_SingleEnded(0);  // Read from channel 0 of the first ADS1115
  adc1 = ads1115_default.readADC_SingleEnded(1);
  adc2 = ads1115_default.readADC_SingleEnded(2);

  // adc4 = ads1115_addr.readADC_SingleEnded(0);        // Read from channel 0 of the second ADS1115
  // adc5 = ads1115_addr.readADC_SingleEnded(1);
  // adc6 = ads1115_addr.readADC_SingleEnded(2);

  Serial.print("Default ADS1115 (0x48) Channel 0: "); Serial.println(adc0);
  Serial.print("Default ADS1115 (0x48) Channel 1: "); Serial.println(adc1);
  Serial.print("Default ADS1115 (0x48) Channel 2: "); Serial.println(adc2);

  // Serial.print("ADDR ADS1115 (0x49) Channel 4: "); Serial.println(adc4);
  // Serial.print("ADDR ADS1115 (0x49) Channel 5: "); Serial.println(adc5);
  // Serial.print("ADDR ADS1115 (0x49) Channel 6: "); Serial.println(adc6);

  delay(1000);
}

