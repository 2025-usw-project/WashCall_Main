#include "StateManager.h"
#include "config.h"

// 생성자: 초기 상태 설정 및 데이터 초기화
StateManager::StateManager() {
  currentState = OFF;
  lastState = OFF;
  stopStartTime = 0;
  // [추가됨] 상태 시작 시간 초기화
  stateStartTime = 0; 
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

// 1초마다 실행되는 메인 로직
void StateManager::update() {
  // 1. 캘리브레이션 모드 처리
  if (currentState == CALIBRATING) {
    calibration->runCalibrationCycle(*sensor);
    return;
  }

  // 2. 일반 모드 처리
  VibrationData data = sensor->getVibration();
  MachineState nextState = currentState; // 다음 예상 상태
  bool justFinished = false; // FINISHED로 방금 변경되었는지 확인
  
  // [추가됨] 서버로 보낼 경과 시간 (초 단위)
  unsigned long timeElapsed = 0; 

  // 2-1. 진동 세기 기준으로 상태 판단 및 데이터 기록
  if (data.magnitude > calibration->getSpinThreshold()) { // 탈수 기준점보다 강함
    if (isPatternMatch(data)) { // 진동 지문 일치 -> SPINNING
      // 새 사이클 시작 감지 (OFF/FINISHED에서 시작)
      if (currentState == OFF || currentState == FINISHED) {
         resetCycleData(); // 새 사이클이므로 이전 기록 초기화
         // [추가됨] 상태 시작 시간 기록
         stateStartTime = millis(); 
      }
      nextState = SPINNING;
      if (data.magnitude > spinMaxMagnitude) {
        spinMaxMagnitude = data.magnitude;
      }
    } else { // 진동 지문 불일치 -> EXTERNAL_VIBRATION
      nextState = EXTERNAL_VIBRATION;
    }
    stopStartTime = 0; // 작동 중이므로 완료 타이머 리셋
  }
  else if (data.magnitude > calibration->getWashThreshold()) { // 세탁 기준점보다 강함 -> WASHING
    // 새 사이클 시작 감지 (OFF/FINISHED에서 시작)
    if (currentState == OFF || currentState == FINISHED) {
       resetCycleData(); // 새 사이클이므로 이전 기록 초기화
       // [추가됨] 상태 시작 시간 기록
       stateStartTime = millis(); 
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
  
  // [추가됨] 현재 상태 경과 시간 계산 (초 단위)
  // WASHING, SPINNING, EXT_VIBE 상태일 때만 경과 시간을 추적
  if (currentState == WASHING || currentState == SPINNING || currentState == EXTERNAL_VIBRATION) {
      // (현재 시간 - 상태 시작 시간) / 1000 (밀리초를 초로 변환)
      timeElapsed = (millis() - stateStartTime) / 1000; 
  }
  // OFF/FINISHED로 상태가 변경될 때 timeElapsed는 0으로 전송됩니다.


  // 3. 상태 변화 감지 및 서버 전송
  if (nextState != lastState) {
    Serial.print("State changed: ");
    Serial.print(stateToString(currentState));
    Serial.print(" -> ");
    Serial.println(stateToString(nextState));

    currentState = nextState; 
    
    // [추가됨] 새로운 사이클이 시작되었을 때 stateStartTime을 다시 기록
    // (예: EXT_VIBE -> WASHING으로 다시 돌아왔을 때)
    if (currentState == WASHING || currentState == SPINNING || currentState == EXTERNAL_VIBRATION) {
      stateStartTime = millis();
    }


    if (currentState != CALIBRATING) {
      // FINISHED 상태 전송 (운영 데이터만 보냄, timeElapsed=0)
      if (justFinished) {
          float washAvgMagnitude = 0;
          if (washVibrationCount > 0) {
              washAvgMagnitude = static_cast<float>(washTotalMagnitude / washVibrationCount);
          }
          // [수정됨] timeElapsed를 0으로 전달 (FINISHED는 경과 시간이 아닌 결과)
          connectivity->sendReport(stateToString(currentState), true, washAvgMagnitude, washMaxMagnitude, spinMaxMagnitude, 0); 
      }
      // 그 외 상태 전송 (WASHING, SPINNING, OFF, EXT_VIBE)
      else {
          // [수정됨] timeElapsed를 함께 전달 (OFF 상태가 되면 timeElapsed는 0으로 전달됨)
          connectivity->sendReport(stateToString(currentState), false, 0, 0, 0, timeElapsed); 
      }
    }

    lastState = currentState;
  }
}

// '진동 지문' 일치 확인 함수
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

// 상태 Enum -> 문자열 변환 함수
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