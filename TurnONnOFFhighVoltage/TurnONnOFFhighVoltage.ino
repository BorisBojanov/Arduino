int Charge_EN_pin = 25;
static const int ADC_pin = 34;
int D2 = 2;
int voltageReading;

// ADC reference / scaling
// ESP32 ADC is 0..4095 counts at 12-bit, but effective full-scale depends on attenuation.
// We'll use attenuation setting + a nominal Vref and then rely on calibration factor.
static const int   ADC_BITS = 12;
static const int   ADC_MAX  = (1 << ADC_BITS) - 1;
// Nominal ADC full-scale voltage with ADC_11db attenuation is ~3.3V (varies!)
static const float ADC_FS_VOLTS_NOMINAL = 3.3;

void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);

  pinMode(Charge_EN_pin, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(ADC_pin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  float voltage = readAdcVolt(ADC_pin);
  Serial.print("The voltageReading is: ");
  Serial.println(voltage);

  if (voltage > 1) {
    digitalWrite(Charge_EN_pin, LOW);
    digitalWrite(D2, LOW);

  } else {
    digitalWrite(Charge_EN_pin, HIGH);
    digitalWrite(D2, HIGH);
  }

}

static float readAdcVolt (int pin){
  const int N = 16;
  uint32_t sum = 0;
  for (int i = 0; i<N; i++){
    sum += analogRead(pin);
    delayMicroseconds(200);
  }
  float raw = (float)sum / (float)N;

  float voltage = ADC_FS_VOLTS_NOMINAL * (raw / (float)ADC_MAX);
  return voltage;
}