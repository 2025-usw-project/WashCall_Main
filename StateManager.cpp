#include "StateManager.h"
#include "config.h"

StateManager::StateManager() {
  currentState = OFF; // 초기 상태는 'OFF'
  lastState = OFF;
  stopStartTime = 0;
}

void StateManager::begin(Calibration &calib, VibrationSensor &sensor, Connectivity &conn) { // 각 모듈 클래스 객체를 참조로 가져오기
  // 외부에서 받은 모듈들의 주소를 내부 변수에 저장하여 연결
  this->calibration = &calib;
  this->sensor = &sensor;
  this->connectivity = &conn;

  // 캘리브레이션이 필요한지 확인하여 초기 상태 결정
  if (calibration->needsCalibration()) {
    currentState = CALIBRATING;
  }
}

// 1초마다 실행되는 메인 판단 로직
void StateManager::update() {
  // 1. 캘리브레이션 모드 처리
  if (currentState == CALIBRATING) {
    calibration->runCalibrationCycle(*sensor);
    // runCalibrationCycle 함수 안에서 재부팅하므로, 아래 코드는 실행되지 않음
    return;
  }

  // 2. 일반 모드 처리
  VibrationData data = sensor->getVibration();

  // 2-1. 진동 세기를 기준으로 1차 판단
  if (data.magnitude > calibration->getSpinThreshold()) { // 탈수 기준점보다 강한 진동
    if (isPatternMatch(data)) { // 진동 지문이 일치하는가?
      currentState = SPINNING;
    } else {
      currentState = EXTERNAL_VIBRATION; // 패턴이 다르면 외부 소음으로 판단
    }
    stopStartTime = 0; // 타이머 리셋
  } 
  else if (data.magnitude > calibration->getWashThreshold()) { // 세탁 기준점보다 강한 진동
    currentState = WASHING;
    stopStartTime = 0; // 타이머 리셋
  } 
  else { // 기준점보다 약한 진동 (멈춤 상태)
    if (currentState == WASHING || currentState == SPINNING) { // 직전까지 작동 중이었다면
      if (stopStartTime == 0) {
        stopStartTime = millis(); // 5분 타이머 시작
      }
      if (millis() - stopStartTime > FINISHED_TIMEOUT_MS) {
        currentState = FINISHED; // 5분 이상 멈춰있으면 '완료'로 최종 판단
      }
    } else if (currentState != FINISHED) {
      currentState = OFF; // 그 외의 경우는 그냥 'OFF'
    }
  }

  // 3. 상태 변화 감지 및 출력
  if (currentState != lastState) {
    Serial.print("State changed: ");
    Serial.println(stateToString(currentState));
    
    // '외부 진동' 상태가 아닐 때만 서버로 상태를 전송
    if(currentState != EXTERNAL_VIBRATION && currentState != CALIBRATING) {
    connectivity->sendState(stateToString(currentState));
    }
    
    lastState = currentState;
  }
}

// '진동 지문'이 일치하는지 확인하는 함수 (외부 진동인지 확인)
bool StateManager::isPatternMatch(VibrationData data) {
  float totalDelta = data.deltaX + data.deltaY + data.deltaZ;
  if (totalDelta == 0) return false;

  float currentX_ratio = data.deltaX / totalDelta;
  float currentY_ratio = data.deltaY / totalDelta;
  float currentZ_ratio = data.deltaZ / totalDelta;

  // 현재 X:Y:Z축의 비율과 저장된 진동 지문 기준점의 차이가 오차 범위 밖이면 외부 소음
  if (abs(currentX_ratio - calibration->getSignatureX()) < PATTERN_MATCH_TOLERANCE &&
      abs(currentY_ratio - calibration->getSignatureY()) < PATTERN_MATCH_TOLERANCE &&
      abs(currentZ_ratio - calibration->getSignatureZ()) < PATTERN_MATCH_TOLERANCE) {
    return true;
  }
  return false;
}

// 상태를 사람이 알아보기 쉬운 문자열로 바꿔주는 함수
String StateManager::stateToString(MachineState state) {
  switch(state) {
    case CALIBRATING: return "CALIBRATING";
    case OFF: return "OFF";
    case WASHING: return "WASHING";
    case SPINNING: return "SPINNING";
    case FINISHED: return "FINISHED";
    case EXTERNAL_VIBRATION: return "EXTERNAL_VIBRATION";
    default: return "UNKNOWN";
  }
}