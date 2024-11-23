#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ezButton.h>  // The library to use for SW pin

Adafruit_MPU6050 mpu;

// IR Sensor Pin
#define IR_SENSOR_PIN D5


void setup() {
  Serial.begin(9600);
  
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

}
void loop() {
  // Read MPU6050 data
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  Serial.print("Accel X: "); Serial.print(a.acceleration.x);
  Serial.print(", Y: "); Serial.print(a.acceleration.y);
  Serial.print(", Z: "); Serial.println(a.acceleration.z);

  Serial.print("Gyro X: "); Serial.print(g.gyro.x);
  Serial.print(", Y: "); Serial.print(g.gyro.y);
  Serial.print(", Z: "); Serial.println(g.gyro.z);
  
  // Read IR sensor
  int irValue = !digitalRead(IR_SENSOR_PIN);
  Serial.println(irValue); 
  Serial.println(irValue ? "Object Detected" : "No Object");
  delay(500);

  
}
