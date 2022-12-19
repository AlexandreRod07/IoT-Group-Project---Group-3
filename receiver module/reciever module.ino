#include <ESP8266WiFi.h>
#include <espnow.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

// MQTT connection details
#define MQTT_HOST "xjanos.top"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:hwu:esp8266: Alex "
#define MQTT_USER "fsj" // no need for authentication, for now
#define MQTT_TOKEN "sijunfang" // no need for authentication, for now
#define MQTT_TOPIC "Alex /evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "Alex /cmd/display/fmt/json"



// Add WiFi connection information
char ssid[] = "fang";  // your network SSID (name)
char pass[] = "123456789";  // your network password

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

StaticJsonDocument<200> Doc1;
JsonObject payload1 = Doc1.to<JsonObject>();
JsonObject BOARD1 = payload1.createNestedObject("board1");
static char msg1[200];

StaticJsonDocument<200> Doc2;
JsonObject payload2 = Doc2.to<JsonObject>();
JsonObject BOARD2 = payload2.createNestedObject("board2");
static char msg2[200];

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
    int ID;
    bool SW;
} struct_message;

// Create a struct_message called myData
struct_message myData;


// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;

// Create an array with all the structures
struct_message boardsStruct[2] = {board1, board2};

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  
  
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
  Serial.println("");
  Serial.println("WiFi Connected");

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Connect to MQTT broker
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);

  } else {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
}


void loop(){
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }

  }

  boardsStruct[myData.ID-1].SW = myData.SW;
  int SWvalue;
  SWvalue = boardsStruct[myData.ID-1].SW;
    
  if (myData.ID ==1 and SWvalue ==0 ){
    
    BOARD1["TESTsw"] = "off";
  } else if (myData.ID == 1 and SWvalue ==1){
    
    BOARD1["TESTsw"] = "on";
  } else if (myData.ID == 2 and SWvalue ==0){
    
    BOARD2["TESTsw"] = "off";
  } else if (myData.ID == 2 and SWvalue ==1){
    
    BOARD2["TESTsw"] = "on";
  }
  serializeJson(Doc1, msg1, 200);
  serializeJson(Doc2, msg2, 200);
  Serial.println(msg1);
  Serial.println(msg2);
  Serial.println();
  delay(500);

    
  if (!mqtt.publish(MQTT_TOPIC, msg1)) {
      Serial.println("MQTT Publish failed");
  }

  
  if (!mqtt.publish(MQTT_TOPIC, msg2)) {
      Serial.println("MQTT Publish failed");
  }

  // Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 10; i++) {
    mqtt.loop();
    delay(10);
  }
  
}