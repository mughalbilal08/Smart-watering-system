#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <Firebase_ESP_Client.h>
#include "ThingSpeak.h"
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

HardwareSerial SerialPort(2);

#define DATABASE_URL "https://soil-moisturizer-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyA0AW_585HWd33xr2hz22Z8ayS0nJpqzQ4"
#define SSID "OnePlus 6T-d5f3"
#define PASSWORD "00407684"
#define USER_EMAIL "bi44309@gmail.com"
#define USER_PASSWORD "muhammad1122"
#define CHANNEL_ID 2395270
#define THINGSPEAK_API_KEY "7BUD2DPYVU8RU91Y"

const char* mqttServer = "broker.mqtt.cool";
const int mqttPort = 1883;
String receivedValue = "-1";
char receivedChars[10];
String uid;

WiFiClient espClient;
WiFiClient thingSpeakClient;
PubSubClient client(espClient);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  SerialPort.begin(9600, SERIAL_8N1, 17, 16);
  connectToWiFi();
  connectToMQTT();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.reconnectWiFi(true);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);
  ThingSpeak.begin(thingSpeakClient);
}
void connectToWiFi() {
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void connectToMQTT() {
  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("2022-CS-19/esp/esp-controller")) {
      Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  client.subscribe("2022-CS-19/esp/button");
  client.setCallback(callback);
}

void publishMessage(char* mqttTopic, char* message) {
  if (client.connected()) {
    client.publish(mqttTopic, message);
    Serial.println("Message published to MQTT");
  } else {
    Serial.println("Failed to publish message. Reconnecting to MQTT...");
    connectToMQTT(); // Reconnect to MQTT
    if (client.connected()) {
      client.publish(mqttTopic, message);
      Serial.println("Message published to MQTT");
    } else {
      Serial.print("Failed to reconnect with state ");
      Serial.println(client.state());
    }
  }
}

void writeToThingSpeak(int level) {
  ThingSpeak.writeField(CHANNEL_ID, 1, level, THINGSPEAK_API_KEY);
}

void writeToFireBase(int level) {
    if (Firebase.ready()) {
    Firebase.RTDB.setInt(&fbdo, "/Moisture/Level", level);
  }
}

void writeToMQTT(char* level) {
  publishMessage("2022-CS-19/esp/reading", level);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String topicStr = topic;
  String payloadStr(reinterpret_cast<char*>(payload), length);
  Serial.print(payloadStr);
  if (payloadStr == "ON") {
    SerialPort.print('1');
    Serial.print('1');
  } else {
    Serial.print('2');
  }
}

void loop() {
  client.loop();
  if (SerialPort.available() > 0) {
    receivedValue = SerialPort.readStringUntil('\n');
    receivedValue.trim();
    receivedValue.toCharArray(receivedChars, 10);
    Serial.println(receivedValue);
    writeToMQTT(receivedChars);
    writeToFireBase(receivedValue.toInt());
    writeToThingSpeak(receivedValue.toInt());
  }
}
