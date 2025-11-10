#ifndef VIBRATIONSENSOR_H
#define VIBRATIONSENSOR_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

// 센서가 최종적으로 계산해낼 데이터의 형태를 정의
struct VibrationData {
  float magnitude; // 종합적인 진동의 세기
  float deltaX;    // X축의 순수 움직임
  float deltaY;    // Y축의 순수 움직임
  float deltaZ;    // Z축의 순수 움직임
};

class VibrationSensor {
  public:
    VibrationSensor();    // 생성자
    bool begin();         // 센서 초기화 함수
    VibrationData getVibration(); // 현재 진동 데이터를 계산하여 반환하는 함수

  private:
    Adafruit_MPU6050 mpu; // MPU6050 라이브러리 객체
    // 이전 측정값을 저장하여 순수 움직임만 계산하기 위한 변수
    float last_ax;
    float last_ay;
    float last_az;
};

#endif