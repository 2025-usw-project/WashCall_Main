#ifndef CALIBRATION_H
#define CALIBRATION_H

#include "VibrationSensor.h" // VibrationSensor의 데이터 형식을 알아야 하므로 포함

class Calibration {
  public:
    Calibration(); // 생성자
    void begin(); // EEPROM을 초기화하고 저장된 기준점을 불러오는 함수
    bool needsCalibration(); // 캘리브레이션이 필요한지 알려주는 함수
    void runCalibrationCycle(VibrationSensor &sensor); // 캘리브레이션(학습)을 실행하는 메인 함수
    
    // 다른 모듈이 기준점 값을 사용할 수 있도록 외부에 값을 알려주는 함수들gg
    float getWashThreshold();
    float getSpinThreshold();
    float getSignatureX();
    float getSignatureY();
    float getSignatureZ();

  private:
    void load(); // EEPROM에서 기준점을 읽어오는 내부 함수
    void save(); // EEPROM에 기준점을 저장하는 내부 함수

    bool isCalibrated_flag; // 캘리브레이션 완료 여부
    float washThreshold;      // 계산된 세탁 기준점
    float spinThreshold;      // 계산된 탈수 기준점
    float signatureX, signatureY, signatureZ; // 계산된 진동 지문
};

#endif