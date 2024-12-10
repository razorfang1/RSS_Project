#include <ESP8266WiFi.h>

// Access Point credentials
const char* ssid = "ESP8266_AP";       // Name of the WiFi network to create
const char* password = "password123";  // Password for the WiFi network

WiFiServer server(80); // Start server on port 80

void setup() {
  Serial.begin(115200);

  // Set up the ESP8266 as an Access Point
  WiFi.softAP(ssid, password);

  // Print the IP address of the Access Point
  Serial.println("Access Point Created!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());

  // Start the server
  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // Check for a new client
  if (client) {
    Serial.println("Client connected!");

    // Send "Hello World" message repeatedly
    while (client.connected()) {
      client.println("Hello World");
      delay(1000); // Send the message every second
    }

    // Disconnect the client after the loop
    client.stop();
    Serial.println("Client disconnected.");
  }
}
