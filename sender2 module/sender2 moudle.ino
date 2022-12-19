#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Adafruit_NeoPixel.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = { 0x84, 0xF3, 0xEB, 0x1B, 0x3F, 0xD6 };

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 1

const int button = 5;
const int ledPin =  4;
int temp = 0;

Adafruit_NeoPixel pixel = Adafruit_NeoPixel(1, ledPin, NEO_RGB + NEO_KHZ800);

typedef struct struct_message {
  int ID;
  bool SW;
} struct_message;


// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 5;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);

  //set the mode of the pins...
  pinMode(button, INPUT);
  pinMode(ledPin, OUTPUT);

  // start neopixel
  pixel.begin();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  wifi_set_channel(2);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}

void loop() {
  temp = digitalRead(button);
  pixel.clear();
  if ((millis() - lastTime) > timerDelay) {
    if (temp == HIGH) {
      Serial.println("pressed");
      myData.ID = BOARD_ID;
      myData.SW = true;

      pixel.setPixelColor(0,165,25,207);
      pixel.show();
      esp_now_send(0, (uint8_t *)&myData, sizeof(myData));
      lastTime = millis();
    }
    else {
      Serial.println("no press");
      myData.ID = BOARD_ID;
      myData.SW = false;
      esp_now_send(0, (uint8_t *)&myData, sizeof(myData));
      lastTime = millis();
    }
  }
}