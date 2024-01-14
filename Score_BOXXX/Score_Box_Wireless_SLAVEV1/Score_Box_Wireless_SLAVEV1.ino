// Include Libraries
#include <esp_now.h>
#include <WiFi.h>
 
// Variables for test data
int int_value;
float float_value;
bool bool_value = true;
 
// MAC Address of responder - edit as required
uint8_t broadcastAddress[] = {0x48, 0xE7, 0x29, 0x98, 0xD7, 0x90};
 
// Define a data structure
typedef struct struct_message {
  char Mode_Value[32];
  bool d;
} struct_message;
 
// Create a structured object
struct_message myData;
 
// Peer info
esp_now_peer_info_t peerInfo;
 
// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
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
 
  // Register the send callback (IMPORTANT)
  // This step must be done for any function you want to become the "Callback"
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {
  // Invert the boolean value
  bool_value = !bool_value;
  
  // Format structured data
  strcpy(myData.Mode_Value, "Welcome to the Workshop!");
  myData.d = bool_value;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.println("Sending error");
  }
  delay(2000);
}