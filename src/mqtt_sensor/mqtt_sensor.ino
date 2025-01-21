#include <ESP8266WiFi.h>
#include <WiFiClient.h>         // Use WiFiClient (not WiFiClientSecure)
#include <PubSubClient.h>       // MQTT client library

// Replace with your Wi-Fi credentials
const char* ssid = "Rohit";
const char* password = "rohit123";

// MQTT Broker settings
const char* mqtt_server = "192.168.137.252";  // IP of your Mosquitto broker (use localhost if on the same machine)
const int mqtt_port = 1883;  // Non-secure port for MQTT
const char* mqtt_user = "username";  // MQTT username (optional)
const char* mqtt_pass = "password";  // MQTT password (optional)

// WiFi and MQTT Clients
WiFiClient wifiClient;  // Standard WiFiClient (no SSL/TLS needed)
PubSubClient client(wifiClient);

// MQTT callback function
void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(" with message: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    Serial.print(WiFi.status());
    Serial.print(WL_CONNECTED);
  }
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up the MQTT client
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
    
      Serial.println("connected");
      client.subscribe("esp8266/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  // Reconnect if disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Send a test message
  client.publish("esp8266/topic", "Hello from ESP8266.....Rohit is Baadshah");
  delay(5000);  // Wait for 5 seconds before sending the next message
}

