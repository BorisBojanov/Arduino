/*
Goals
This is the Reciever Scetch
It will recieve a value that is Boolean or an int16_t (to be determined)
Need a way to Pair two ESP32s as slvaes
It will then make the LEDs turn on. 
It will also check for the lock out time.
    Measuring time between signals recieved
It will also check for the reset time.
    This will check if enough time has passed after a light being turned on and then turn them off

*/

// Include Libraries
#include <esp_now.h>
#include <WiFi.h>

// Define a data structure
typedef struct struct_message {
  char Mode_Value[32];  
  bool d;
} struct_message;

// Create a structured object
struct_message myData;

// Callback function executed when data is received
// Keep Callback functions small and treat them as ISRs
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("Mode Value:");
  Serial.println(myData.Mode_Value);
  Serial.print("Float Value: ");
  Serial.println(myData.d);
  Serial.println();
}


void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  
  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
 
  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  
}