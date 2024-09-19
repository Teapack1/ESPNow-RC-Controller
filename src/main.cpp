#include <esp_now.h>
#include <WiFi.h>

// Movement Joystick
#define JOY1_X 34
#define JOY1_Y 35

// Claw Joystick
#define JOY2_X 32
#define JOY2_Y 33
#define JOY2_Z 25  // Assuming a button for the Z axis

// MAC Address of the toy - edit as required
//uint8_t toyAddress[] = {0x24, 0x0A, 0xC4, 0x8B, 0x6C, 0xE8}; //DEVKITV1
uint8_t toyAddress[] = {0xFC, 0xB4, 0x67, 0x56, 0xDA, 0x50}; //car D1 mini

// Define a data structure 
typedef struct struct_message {
  int joy1_x;
  int joy1_y;
  int joy2_x;
  int joy2_y;
  int joy2_z;
} struct_message;

// Create a structured object
struct_message myData;

// Peer info
esp_now_peer_info_t peerInfo;

/*
// Calibration offsets
const int JOY1_X_OFFSET = 2048 - 1831;
const int JOY1_Y_OFFSET = 2048 - 1858;
const int JOY2_X_OFFSET = 2048 - 1852;
const int JOY2_Y_OFFSET = 2048 - 1829;
*/


// Calibration offsets
const int JOY1_X_OFFSET = 2048 - 2180;
const int JOY1_Y_OFFSET = 2048 - 2150;
const int JOY2_X_OFFSET = 2048 - 1980;
const int JOY2_Y_OFFSET = 2048 - 2140;




// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  // Set up Serial Monitor
  Serial.begin(115200);
  delay(1000);  // Delay for stabilization

  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, toyAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}


int scaleValue(int value, int offset, int minValue, int maxValue) {
  int midpoint = (maxValue + minValue) / 2;
  if ((value) <= midpoint) {
    return map(value, minValue, midpoint, 0, 2048+ offset);
  } else {
    return map(value, (midpoint+1), maxValue, 2049-offset, 4095);
  }
}



void loop() {
  // Read the joystick values
  int raw_joy1_x_value = analogRead(JOY1_X);
  int raw_joy1_y_value = analogRead(JOY1_Y);
  int raw_joy2_x_value = analogRead(JOY2_X);
  int raw_joy2_y_value = analogRead(JOY2_Y);
  //int joy2_z_value = digitalRead(JOY2_Z);

    // Write the values to serial monitor for debugging
  Serial.print("raw_M" + String(raw_joy1_x_value) + "," + String(raw_joy1_y_value) + "\n"); // Movement values
  Serial.print("raw_C" + String(raw_joy2_x_value) + "," + String(raw_joy2_y_value) + "\n"); // Claw values



  // Apply scaling based on calibration offsets
  
  int joy1_x_value = scaleValue(raw_joy1_x_value, JOY1_X_OFFSET, 3200, 900);
  int joy1_y_value = scaleValue(raw_joy1_y_value, JOY1_Y_OFFSET, 2850, 650);
  int joy2_x_value = scaleValue(raw_joy2_x_value, JOY2_X_OFFSET, 460, 3100);
  int joy2_y_value = scaleValue(raw_joy2_y_value, JOY2_Y_OFFSET, 2750, 500);


  // Map the scaled joystick values for the motors
  myData.joy1_x = map(joy1_x_value, 0, 4095, -255, 255);
  myData.joy1_y = map(joy1_y_value, 0, 4095, -255, 255);
  myData.joy2_x = map(joy2_x_value, 0, 4095, -255, 255);
  myData.joy2_y = map(joy2_y_value, 0, 4095, -255, 255);
  //myData.joy2_z = joy2_z_value;




  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(toyAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK) {
    Serial.println("Sending confirmed");
  } else {
    Serial.println("Sending error");
  }
  Serial.println("0x24, 0x0A, 0xC4, 0x8B, 0x6C, 0xE8"); //skid
  Serial.println("0xFC, 0xB4, 0x67, 0x56, 0xDA, 0x50"); //car


  // Write the values to serial monitor for debugging
  Serial.print("M" + String(myData.joy1_x) + "," + String(myData.joy1_y) + "\n"); // Movement values
  Serial.print("C" + String(myData.joy2_x) + "," + String(myData.joy2_y) + "\n"); // Claw values

  // Write the raw values to serial monitor for debugging
  Serial.print("M" + String(joy1_x_value) + "," + String(joy1_y_value) + "\n"); // Movement values
  Serial.print("C" + String(joy2_x_value) + "," + String(joy2_y_value) + "\n"); // Claw values

  delay(1);
}