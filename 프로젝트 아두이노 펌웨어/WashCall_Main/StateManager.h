#ifndef STATEMANAGER_H
#define STATEMANAGER_H

#include "VibrationSensor.h"
#include "Calibration.h"
#include "Connectivity.h" // 나중에 사용할 통신 모듈도 미리 포함

// 기기가 가질 수 있는 모든 상태를 명확하게 이름으로 정의 (enum)
enum MachineState {
  CALIBRATING, // 캘리브레이션(학습) 중
  OFF,         // 작동 멈춤
  WASHING,     // 세탁/헹굼 중
  SPINNING,    // 탈수 중
  FINISHED,    // 세탁 완료 (설정된 시간 경과)
  EXTERNAL_VIBRATION // 외부 진동으로 의심될 때
};

class StateManager {
  public:
    StateManager();
    // 다른 부품(모듈)들을 연결하여 초기화하는 함수
    void begin(Calibration &calib, VibrationSensor &sensor, Connectivity &conn);
    // 1초마다 호출되어 현재 상태를 계속 업데이트하는 메인 함수
    void update(VibrationData data);

  private:
    // 외부 모듈의 주소를 저장할 포인터 변수
    Calibration* calibration;
    VibrationSensor* sensor;
    Connectivity* connectivity;

    MachineState currentState; // 현재 상태를 저장하는 변수
    MachineState lastState;    // 바로 이전 상태를 저장하는 변수

    unsigned long stopStartTime; // 진동이 멈추기 시작한 시간을 기록하는 타이머

    // --- 운영 데이터 기록용 변수들 ---
    double washTotalMagnitude;  // WASHING 상태 동안 진동 값 총합
    long washVibrationCount;    // WASHING 상태 동안 진동 측정 횟수
    float washMaxMagnitude;     // WASHING 상태 동안 최대 진동 값
    float spinMaxMagnitude;     // SPINNING 상태 동안 최대 진동 값
    // --- ---

    // 현재 상태를 사람이 알아보기 쉬운 문자열로 바꿔주는 함수
    String stateToString(MachineState state);
    // 진동 지문이 일치하는지 확인하는 함수
    bool isPatternMatch(VibrationData data);
    // 새 사이클 시작 시 기록 초기화 함수
    void resetCycleData();
};

#endif
