#include "StateManager.h"
#include "config.h"

// 생성자: 초기 상태 설정 및 데이터 초기화
StateManager::StateManager() {
  currentState = OFF;
  lastState = OFF;
  stopStartTime = 0;
  resetCycleData();
}

// 모듈 연결 및 초기 상태 결정
void StateManager::begin(Calibration &calib, VibrationSensor &sensor, Connectivity &conn) {
  this->calibration = &calib;
  this->sensor = &sensor;
  this->connectivity = &conn;

  // 캘리브레이션 필요 여부 확인
  if (calibration->needsCalibration()) {
    currentState = CALIBRATING;
    lastState = CALIBRATING; // lastState도 초기화
  }
}

// 새 사이클 데이터 기록 변수 초기화 함수
void StateManager::resetCycleData() {
  washTotalMagnitude = 0;
  washVibrationCount = 0;
  washMaxMagnitude = 0;
  spinMaxMagnitude = 0;
}

// [수정됨] 1초마다 실행되는 메인 로직 (data를 파라미터로 받음)
void StateManager::update(VibrationData data) {
  // 1. 캘리브레이션 모드 처리
  if (currentState == CALIBRATING) {
    calibration->runCalibrationCycle(*sensor);
    return;
  }

  // 2. 일반 모드 처리
  MachineState nextState = currentState; // 다음 예상 상태
  bool justFinished = false; // FINISHED로 방금 변경되었는지 확인

  // 2-1. 진동 세기 기준으로 상태 판단 및 데이터 기록
  if (data.magnitude > calibration->getSpinThreshold()) { // 탈수 기준점(1.2)보다 강함
    
    // [수정됨] 진동 지문(isPatternMatch) 검사 및 EXT_VIBE 로직을 삭제합니다.
    // if (isPatternMatch(data)) { 
      
      // 새 사이클 시작 감지 (OFF/FINISHED에서 시작)
      if (currentState == OFF || currentState == FINISHED) {
         resetCycleData(); // 새 사이클이므로 이전 기록 초기화
      }
      nextState = SPINNING;
      if (data.magnitude > spinMaxMagnitude) {
        spinMaxMagnitude = data.magnitude;
      }
    // } else { 
    //   nextState = EXTERNAL_VIBRATION; 
    // } 

    stopStartTime = 0; // 작동 중이므로 완료 타이머 리셋
  }
  else if (data.magnitude > calibration->getWashThreshold()) { // 세탁 기준점(0.2)보다 강함 -> WASHING
    // 새 사이클 시작 감지 (OFF/FINISHED에서 시작)
    if (currentState == OFF || currentState == FINISHED) {
       resetCycleData(); // 새 사이클이므로 이전 기록 초기화
    }
    nextState = WASHING;
    washTotalMagnitude += data.magnitude;
    washVibrationCount++;
    if (data.magnitude > washMaxMagnitude) {
      washMaxMagnitude = data.magnitude;
    }
    stopStartTime = 0; // 작동 중이므로 완료 타이머 리셋
  }
  else { // 기준점보다 약함 (멈춤 상태 판단 시작)
    if (currentState == WASHING || currentState == SPINNING) {
      if (stopStartTime == 0) {
        stopStartTime = millis(); // 타이머 시작
      }
      if (millis() - stopStartTime > FINISHED_TIMEOUT_MS) {
        if (currentState != FINISHED) { // FINISHED 상태가 아니었다면
            nextState = FINISHED;
            justFinished = true; // 방금 완료됨 플래그 설정
        }
      }
      else {
          nextState = currentState; // 상태 유지
      }
    }
    else if (currentState != FINISHED) {
      nextState = OFF;
    }
    else {
        nextState = FINISHED;
    }
  }

  // 3. 상태 변화 감지 및 서버 전송
  if (nextState != lastState) {
    Serial.print("State changed: ");
    Serial.print(stateToString(currentState));
    Serial.print(" -> ");
    Serial.println(stateToString(nextState));

    currentState = nextState; 
    


    if (currentState != CALIBRATING) {
      // FINISHED 상태 전송 (운영 데이터만 보냄)
      if (justFinished) {
          float washAvgMagnitude = 0;
          if (washVibrationCount > 0) {
              washAvgMagnitude = static_cast<float>(washTotalMagnitude / washVibrationCount);
          }
          connectivity->sendReport(stateToString(currentState), true, washAvgMagnitude, washMaxMagnitude, spinMaxMagnitude); 
      }
      // 그 외 상태 전송 (WASHING, SPINNING, OFF)
      else {
          // [수정됨] EXT_VIBE 상태는 이제 발생하지 않음
          if (currentState != EXTERNAL_VIBRATION) {
            connectivity->sendReport(stateToString(currentState), false, 0, 0, 0); 
          }
      }
    }

    lastState = currentState;
  }
}

// [유지됨] '진동 지문' 일치 확인 함수 (호출되지는 않지만 삭제할 필요 없음)
bool StateManager::isPatternMatch(VibrationData data) {
  float totalDelta = data.deltaX + data.deltaY + data.deltaZ;
  if (totalDelta == 0) return false;

  float currentX_ratio = data.deltaX / totalDelta;
  float currentY_ratio = data.deltaY / totalDelta;
  float currentZ_ratio = data.deltaZ / totalDelta;

  if (abs(currentX_ratio - calibration->getSignatureX()) < PATTERN_MATCH_TOLERANCE &&
      abs(currentY_ratio - calibration->getSignatureY()) < PATTERN_MATCH_TOLERANCE &&
      abs(currentZ_ratio - calibration->getSignatureZ()) < PATTERN_MATCH_TOLERANCE) {
    return true;
  }
  return false;
}

// [유지됨] 상태 Enum -> 문자열 변환 함수
String StateManager::stateToString(MachineState state) {
  switch(state) {
    case CALIBRATING: return "CALIBRATING";
    case OFF: return "OFF";
    case WASHING: return "WASHING";
    case SPINNING: return "SPINNING";
    case FINISHED: return "FINISHED";
    case EXTERNAL_VIBRATION: return "EXT_VIBE";
    default: return "UNKNOWN";
  }
}