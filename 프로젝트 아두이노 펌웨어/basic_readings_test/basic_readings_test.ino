#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
float last_ax = 0, last_ay = 0, last_az = 0;

void setup(void) {
  Serial.begin(115200);
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) { delay(10); }
  }
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  last_ax = a.acceleration.x;
  last_ay = a.acceleration.y;
  last_az = a.acceleration.z;
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 이전 값과의 차이를 이용해 순수 '움직임(진동)'만 측정
  float vibration_magnitude = abs(a.acceleration.x - last_ax) + abs(a.acceleration.y - last_ay) + abs(a.acceleration.z - last_az);
  
  Serial.println(vibration_magnitude);

  last_ax = a.acceleration.x;
  last_ay = a.acceleration.y;
  last_az = a.acceleration.z;
  
  delay(1000); // 1초에 한 번씩 측정
}