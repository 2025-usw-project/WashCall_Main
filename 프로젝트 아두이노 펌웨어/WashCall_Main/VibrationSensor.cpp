#include "config.h"
#include "VibrationSensor.h"

// 생성자 (일단 비워둠)
VibrationSensor::VibrationSensor() {
}

// 센서를 시작하고 초기값을 설정하는 함수
bool VibrationSensor::begin() {
  Wire.begin(MPU_SDA_PIN, MPU_SCL_PIN);
  // 센서 연결 시도
  if (!mpu.begin()) {
    Serial.println("MPU6050 Not Found");
    return false;
  }
  
  // 가속도계 측정 범위를 설정하는 함수 (세탁기의 강한 진동을 감지하기에 적합)
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  
  // 최초의 '이전 값'을 현재 값으로 설정하여 첫 계산의 오류를 방지
  delay(100);
  sensors_event_t a, g, temp; // a는 가속도, g는 자이로, temp는 온도 
  mpu.getEvent(&a, &g, &temp); // 센서에서 최신 측정값을 가져오는 함수 (우린 사실상 가속도만 필요)
  last_ax = a.acceleration.x; // 가속도에서 x축만 
  last_ay = a.acceleration.y; // 가속도에서 y축만 
  last_az = a.acceleration.z; // 가속도에서 z축만 
  
  return true;
}

// 현재 진동 데이터를 계산하는 함수
VibrationData VibrationSensor::getVibration() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // 반환할 데이터 구조체 생성
  VibrationData data;

  // 이전 값과의 차이를 계산하여 순수한 움직임(진동)만 추출
  data.deltaX = abs(a.acceleration.x - last_ax);
  data.deltaY = abs(a.acceleration.y - last_ay);
  data.deltaZ = abs(a.acceleration.z - last_az);
  
  // 각 축의 움직임을 모두 더해 종합적인 진동 세기(magnitude) 계산
  data.magnitude = data.deltaX + data.deltaY + data.deltaZ;

  // 다음 계산을 위해 현재 값을 '이전 값'으로 저장
  last_ax = a.acceleration.x;
  last_ay = a.acceleration.y;
  last_az = a.acceleration.z;

  // 최종 계산된 데이터 반환
  return data;
}