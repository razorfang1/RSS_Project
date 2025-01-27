#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ezButton.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

// Capacitive Touch Sensor Pin
const int ain = A0;
int inputVal = 0;
bool touchDetected = false;

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
#define IR_SENSOR_PIN1 0
#define IR_SENSOR_PIN2 14

// Rotary Encoder Pins
// #define CLK_PIN 13  // GPIO D7
// #define DT_PIN 12   // GPIO D6
// #define SW_PIN 15   // GPIO D5
// #define DIRECTION_CW 0   // clockwise direction
// #define DIRECTION_CCW 1  // counter-clockwise direction

// int counter = 0;
// int direction = DIRECTION_CW;
// int CLK_state;
// int prev_CLK_state;

// // Rotary encoder button
// ezButton button(SW_PIN);

// Wi-Fi and MQTT Clients
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// Moving average filter variables
#define FILTER_SIZE 10
float gyroX_buffer[FILTER_SIZE] = {0};
float gyroY_buffer[FILTER_SIZE] = {0};
float gyroZ_buffer[FILTER_SIZE] = {0};
int filter_index = 0;

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
  pinMode(IR_SENSOR_PIN1, INPUT);
  pinMode(IR_SENSOR_PIN2, INPUT);

  // Initialize Rotary Encoder pins with internal pull-ups
  // pinMode(CLK_PIN, INPUT_PULLUP);
  // pinMode(DT_PIN, INPUT_PULLUP);
  // button.setDebounceTime(50);

  // Read the initial state of the rotary encoder's CLK pin
  // prev_CLK_state = digitalRead(CLK_PIN);
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

float calculateAverage(float* buffer, int size) {
  float sum = 0;
  for (int i = 0; i < size; i++) {
    sum += buffer[i];
  }
  return sum / size;
}

// void handleRotaryEncoder() {
//   // Read the current state of the rotary encoder's CLK pin
//   CLK_state = digitalRead(CLK_PIN);
  
//   // Check for state change (rising edge)
//   if (CLK_state != prev_CLK_state) {
//     if (digitalRead(DT_PIN) == CLK_state) {
//         counter++;  // Clockwise direction
//         direction = DIRECTION_CW;
//     } else {
//         counter--;  // Counter-clockwise direction
//         direction = DIRECTION_CCW;
//     }

//     // Publish rotary encoder data every loop
//     String directionStr = (direction == DIRECTION_CW) ? "CLOCKWISE" : "ANTICLOCKWISE";
//     String rotaryData = "Direction: " + directionStr + ", Count: " + String(counter);
//     client.publish("esp8266/rotary", rotaryData.c_str());

//     // Print rotary encoder data
//     Serial.println("MQTT Topic: esp8266/rotary - Data: " + rotaryData);

//     prev_CLK_state = CLK_state;  // Update previous state
//     delay(10); // Debounce delay
//   }

//   // Publish rotary encoder data every loop
//   String directionStr = (direction == DIRECTION_CW) ? "CLOCKWISE" : "ANTICLOCKWISE";
//   String rotaryData = "Direction: " + directionStr + ", Count: " + String(counter);
//   client.publish("esp8266/rotary", rotaryData.c_str());

//   // Print rotary encoder data
//   Serial.println("MQTT Topic: esp8266/rotary - Data: " + rotaryData);

//   // Rotary encoder button press
//   button.loop();
//   if (button.isPressed()) {
//     Serial.println("The button is pressed");
//     client.publish("esp8266/button", "Pressed");
//     Serial.println("MQTT Topic: esp8266/button - Data: Pressed");
//   }
// }

void loop() {
  // Reconnect if disconnected
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Call the rotary encoder handler function
  // handleRotaryEncoder();

  // Update moving average buffers
  gyroX_buffer[filter_index] = g.gyro.z;
  gyroY_buffer[filter_index] = g.gyro.y;
  gyroZ_buffer[filter_index] = g.gyro.x;
  filter_index = (filter_index + 1) % FILTER_SIZE;

  // Calculate filtered gyroscope values
  float filtered_gyroX = calculateAverage(gyroX_buffer, FILTER_SIZE);
  float filtered_gyroY = calculateAverage(gyroY_buffer, FILTER_SIZE);
  float filtered_gyroZ = calculateAverage(gyroZ_buffer, FILTER_SIZE);

  // Publish accelerometer, gyroscope, and temperature data to MQTT
  String accelData = "Accel X: " + String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z);
  String gyroData = String(filtered_gyroX, 2) + "," + String(filtered_gyroY, 2);
  String camData = String(filtered_gyroZ+0.06, 2);
  String tempData = "Temperature: " + String(temp.temperature) + " °C";

  client.publish("esp8266/accel", accelData.c_str());
  client.publish("esp8266/gyro", gyroData.c_str());
  client.publish("esp8266/cam", camData.c_str());
  client.publish("esp8266/temp", tempData.c_str());

  // Print accelerometer, gyroscope, and temperature data to Serial Monitor
  Serial.println("MQTT Topic: esp8266/accel - Data: " + accelData);
  Serial.println("MQTT Topic: esp8266/gyro - Data: " + gyroData);
  Serial.println("MQTT Topic: esp8266/cam - Data: " + camData);
  Serial.println("MQTT Topic: esp8266/temp - Data: " + tempData);

  // Read IR sensor 1 (For door)
  int irValue1 = !digitalRead(IR_SENSOR_PIN1);
  String irStatus1 = irValue1 ? "Object Detected" : "No Object";
  client.publish("esp8266/ir", irStatus1.c_str());

  // Print IR sensor 1 topic
  Serial.println("MQTT Topic: esp8266/ir - Data: " + irStatus1);

  // Read IR sensor 2 (For running)
  int irValue2 = !digitalRead(IR_SENSOR_PIN2);
  String irStatus2 = irValue2 ? "true" : "false";
  client.publish("esp8266/run", irStatus2.c_str());

  // Print IR 2 sensor topic
  Serial.println("MQTT Topic: esp8266/run - Data: " + irStatus2);

  // Read capacitive touch sensor
  inputVal = analogRead(ain);  // Read analog value
  touchDetected = (inputVal > 40);

  // Publish touch sensor state
  String touchStatus = touchDetected ? "true" : "false";
  client.publish("esp8266/touch", touchStatus.c_str());

  // Print touch sensor status to Serial Monitor
  Serial.println("MQTT Topic: esp8266/touch - Data: " + touchStatus);

  // Delay to control the publish frequency
  delay(500);
}



// #include <Wire.h>
// #include <Adafruit_MPU6050.h>
// #include <Adafruit_Sensor.h>
// #include <ezButton.h>
// #include <ESP8266WiFi.h>
// #include <WiFiClient.h>
// #include <PubSubClient.h>

// // Capacitive Touch Sensor Pin
// const int ain = A0;
// int inputVal = 0;
// bool touchDetected = false;

// // Replace with your Wi-Fi credentials
// const char* ssid = "Rohit";
// const char* password = "rohit123";

// // MQTT Broker settings
// const char* mqtt_server = "192.168.137.252";
// const int mqtt_port = 1883;
// const char* mqtt_user = "username";
// const char* mqtt_pass = "password";

// // Initialize the MPU6050
// Adafruit_MPU6050 mpu;

// // IR Sensor Pin
// #define IR_SENSOR_PIN 0

// // Rotary Encoder Pins
// #define CLK_PIN 13  // GPIO D7
// #define DT_PIN 12   // GPIO D6
// #define SW_PIN 14   // GPIO D5
// #define DIRECTION_CW 0   // clockwise direction
// #define DIRECTION_CCW 1  // counter-clockwise direction

// int counter = 0;
// int direction = DIRECTION_CW;
// int CLK_state;
// int prev_CLK_state;

// // Rotary encoder button
// ezButton button(SW_PIN);

// // Wi-Fi and MQTT Clients
// WiFiClient wifiClient;
// PubSubClient client(wifiClient);

// // Moving average filter variables
// #define FILTER_SIZE 10
// float gyroX_buffer[FILTER_SIZE] = {0};
// float gyroY_buffer[FILTER_SIZE] = {0};
// int filter_index = 0;

// void setup() {
//   Serial.begin(115200);

//   // Connect to Wi-Fi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.print(".");
//   }
//   Serial.println("WiFi connected");
//   Serial.print("IP Address: ");
//   Serial.println(WiFi.localIP());

//   // Set up the MQTT client
//   client.setServer(mqtt_server, mqtt_port);

//   // Initialize MPU6050
//   if (!mpu.begin()) {
//     Serial.println("Failed to find MPU6050 chip");
//     while (1) {
//       delay(10);
//     }
//   }
//   Serial.println("MPU6050 Found!");

//   // Configure the accelerometer and gyroscope
//   mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
//   mpu.setGyroRange(MPU6050_RANGE_250_DEG);
//   mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

//   // Initialize IR Sensor
//   pinMode(IR_SENSOR_PIN, INPUT);

//   // Initialize Rotary Encoder pins with internal pull-ups
//   pinMode(CLK_PIN, INPUT_PULLUP);
//   pinMode(DT_PIN, INPUT_PULLUP);
//   button.setDebounceTime(50);

//   // Read the initial state of the rotary encoder's CLK pin
//   prev_CLK_state = digitalRead(CLK_PIN);
// }

// void reconnect() {
//   while (!client.connected()) {
//     Serial.print("Attempting MQTT connection...");
//     if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
//       Serial.println("connected");
//     } else {
//       Serial.print("failed, rc=");
//       Serial.print(client.state());
//       Serial.println(" try again in 5 seconds");
//       delay(5000);
//     }
//   }
// }

// float calculateAverage(float* buffer, int size) {
//   float sum = 0;
//   for (int i = 0; i < size; i++) {
//     sum += buffer[i];
//   }
//   return sum / size;
// }

// void loop() {
//   // Reconnect if disconnected
//   if (!client.connected()) {
//     reconnect();
//   }
//   client.loop();

//   // Read MPU6050 data
//   sensors_event_t a, g, temp;
//   mpu.getEvent(&a, &g, &temp);

//   // Update moving average buffers
//   gyroX_buffer[filter_index] = g.gyro.z;
//   gyroY_buffer[filter_index] = g.gyro.y;
//   filter_index = (filter_index + 1) % FILTER_SIZE;

//   // Calculate filtered gyroscope values
//   float filtered_gyroX = calculateAverage(gyroX_buffer, FILTER_SIZE);
//   float filtered_gyroY = calculateAverage(gyroY_buffer, FILTER_SIZE);

//   // Publish accelerometer, gyroscope, and temperature data to MQTT
//   String accelData = "Accel X: " + String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z);
//   String gyroData = String(filtered_gyroX, 2) + "," + String(filtered_gyroY, 2);
//   String tempData = "Temperature: " + String(temp.temperature) + " °C";

//   client.publish("esp8266/accel", accelData.c_str());
//   client.publish("esp8266/gyro", gyroData.c_str());
//   client.publish("esp8266/temp", tempData.c_str());

//   // Print accelerometer, gyroscope, and temperature data to Serial Monitor
//   Serial.println("MQTT Topic: esp8266/accel - Data: " + accelData);
//   Serial.println("MQTT Topic: esp8266/gyro - Data: " + gyroData);
//   Serial.println("MQTT Topic: esp8266/temp - Data: " + tempData);

//   // Read IR sensor
//   int irValue = !digitalRead(IR_SENSOR_PIN);
//   String irStatus = irValue ? "Object Detected" : "No Object";
//   client.publish("esp8266/ir", irStatus.c_str());

//   // Print IR sensor topic
//   Serial.println("MQTT Topic: esp8266/ir - Data: " + irStatus);
  
//   button.loop();
//   CLK_state = digitalRead(CLK_PIN);

//   // Detect change in CLK state
//   if (CLK_state != prev_CLK_state) {
//     if (digitalRead(DT_PIN) == CLK_state) {  // Check DT pin state relative to CLK
//       counter++;  // Clockwise direction
//       direction = DIRECTION_CW;
//     } else {
//       counter--;  // Counter-clockwise direction
//       direction = DIRECTION_CCW;
//     }

//     String directionStr = (direction == DIRECTION_CW) ? "CLOCKWISE" : "ANTICLOCKWISE";
//     Serial.print("Rotary Encoder:: direction: ");
//     Serial.print(directionStr);
//     Serial.print(" - count: ");
//     Serial.println(counter);

//     // Publish rotary encoder data
//     String rotaryData = "Direction: " + directionStr + ", Count: " + String(counter);
//     client.publish("esp8266/rotary", rotaryData.c_str());

//     // Print rotary encoder topic
//     Serial.println("MQTT Topic: esp8266/rotary - Data: " + rotaryData);
//   }

//   prev_CLK_state = CLK_state;  // Update previous CLK state

//   // Rotary encoder button press
//   if (button.isPressed()) {
//     Serial.println("The button is pressed");
//     client.publish("esp8266/button", "Pressed");

//     // Print button press topic
//     Serial.println("MQTT Topic: esp8266/button - Data: Pressed");
//   }

//   // Read capacitive touch sensor
//   inputVal = analogRead(ain); // Read analog value
//   if (inputVal > 40) {
//     touchDetected = true;
//   } else {
//     touchDetected = false;
//   }

//   // Publish touch sensor state
//   String touchStatus = touchDetected ? "true" : "false";
//   client.publish("esp8266/touch", touchStatus.c_str());

//   // Print touch sensor status to Serial Monitor
//   Serial.println("MQTT Topic: esp8266/touch - Data: " + touchStatus);

//   // Delay to control the publish frequency
//   delay(500);
// }


