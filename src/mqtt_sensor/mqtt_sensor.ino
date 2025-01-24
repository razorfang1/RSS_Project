#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ezButton.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Rohit";
const char* password = "rohit123";

// MQTT Broker settings
const char* mqtt_server = "192.168.137.252";
const int mqtt_port = 1883;
const char* mqtt_user = "username";
const char* mqtt_pass = "password";

// Initialize the MPU6050
Adafruit_MPU6050 mpu;

// IR Sensor Pin
#define IR_SENSOR_PIN 0

// Rotary Encoder Pins
#define CLK_PIN 13  // GPIO D7
#define DT_PIN 12   // GPIO D6
#define SW_PIN 14   // GPIO D5
#define DIRECTION_CW 0
#define DIRECTION_CCW 1

int counter = 0;
int direction = DIRECTION_CW;
int CLK_state;
int prev_CLK_state;

// Rotary encoder button
ezButton button(SW_PIN);

// Wi-Fi and MQTT Clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Set up the MQTT client
  client.setServer(mqtt_server, mqtt_port);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");

  // Configure the accelerometer and gyroscope
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize IR Sensor
  pinMode(IR_SENSOR_PIN, INPUT);

  // Initialize Rotary Encoder pins
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);

  // Read the initial state of the rotary encoder's CLK pin
  prev_CLK_state = digitalRead(CLK_PIN);
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
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

  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Publish accelerometer and gyroscope data to MQTT
  String accelData = "Accel X: " + String(a.acceleration.x) + ", Y: " + String(a.acceleration.y) + ", Z: " + String(a.acceleration.z);
  String gyroData = "Gyro X: " + String(g.gyro.x) + ", Y: " + String(g.gyro.y) + ", Z: " + String(g.gyro.z);
  client.publish("esp8266/accel", accelData.c_str());
  client.publish("esp8266/gyro", gyroData.c_str());

  // Print accelerometer and gyroscope topics
  Serial.println("MQTT Topic: esp8266/accel - Data: " + accelData);
  Serial.println("MQTT Topic: esp8266/gyro - Data: " + gyroData);

  // Read IR sensor
  int irValue = !digitalRead(IR_SENSOR_PIN);
  String irStatus = irValue ? "Object Detected" : "No Object";
  client.publish("esp8266/ir", irStatus.c_str());

  // Print IR sensor topic
  Serial.println("MQTT Topic: esp8266/ir - Data: " + irStatus);

  // Rotary encoder logic
  button.loop();
  CLK_state = digitalRead(CLK_PIN);
  if (CLK_state != prev_CLK_state && CLK_state == HIGH) {
    if (digitalRead(DT_PIN) == HIGH) {
      counter--;
      direction = DIRECTION_CCW;
    } else {
      counter++;
      direction = DIRECTION_CW;
    }

    String directionStr = (direction == DIRECTION_CW) ? "CLOCKWISE" : "ANTICLOCKWISE";
    Serial.print("Rotary Encoder:: direction: ");
    Serial.print(directionStr);
    Serial.print(" - count: ");
    Serial.println(counter);

    // Publish rotary encoder data
    String rotaryData = "Direction: " + directionStr + ", Count: " + String(counter);
    client.publish("esp8266/rotary", rotaryData.c_str());

    // Print rotary encoder topic
    Serial.println("MQTT Topic: esp8266/rotary - Data: " + rotaryData);
  }
  prev_CLK_state = CLK_state;

  // Rotary encoder button press
  if (button.isPressed()) {
    Serial.println("The button is pressed");
    client.publish("esp8266/button", "Pressed");

    // Print button press topic
    Serial.println("MQTT Topic: esp8266/button - Data: Pressed");
  }

  // Delay to control the publish frequency
  delay(500);
}