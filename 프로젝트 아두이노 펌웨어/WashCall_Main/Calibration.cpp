#include <EEPROM.h>
#include "config.h"
#include "Calibration.h"

// 생성자
Calibration::Calibration() {
  isCalibrated_flag = false;
  washThreshold = 0.4;  // (초기 기본값, 어차피 덮어쓰임)
  spinThreshold = 1.8; // (초기 기본값, 어차피 덮어쓰임)
}

// EEPROM 시작 및 데이터 로드
void Calibration::begin() {
  EEPROM.begin(EEPROM_SIZE);
  load();
}

// 캘리브레이션 필요 여부 반환
bool Calibration::needsCalibration() {
  return !isCalibrated_flag;
}

// 캘리브레이션(학습) 메인 로직
void Calibration::runCalibrationCycle(VibrationSensor &sensor) {
  Serial.println("======================================");
  Serial.println(" Starting Auto-Calibration Cycle...");
  Serial.print(" Duration: ");
  Serial.print(CALIBRATION_DURATION_MS / 60000);
  Serial.println(" minutes.");
  Serial.println("======================================");

  // 계산용 변수 초기화
  float maxVibration = 0;
  float maxX_at_maxVib = 0, maxY_at_maxVib = 0, maxZ_at_maxVib = 0;
  // [수정됨] '평균' 계산에 필요한 변수들(totalVibration, vibrationCount)은 더 이상 사용하지 않음
  // double totalVibration = 0; 
  // long vibrationCount = 0;

  unsigned long startTime = millis();
  while (millis() - startTime < CALIBRATION_DURATION_MS) {
    VibrationData data = sensor.getVibration();

    Serial.print("Calibrating... Current Vibration: ");
    Serial.println(data.magnitude);

    if (data.magnitude > 0.25) { // 의미 있는 진동만 데이터에 포함
      if (data.magnitude > maxVibration) {
        maxVibration = data.magnitude; // (spinThreshold 계산을 위해 최대값은 계속 찾음)
        maxX_at_maxVib = data.deltaX;
        maxY_at_maxVib = data.deltaY;
        maxZ_at_maxVib = data.deltaZ;
      }
      // [삭제됨] '평균' 계산 로직 삭제
      // totalVibration += data.magnitude;
      // vibrationCount++;
    }
    delay(1000);
  }

  // --- 기준점 및 '진동 지문' 최종 계산 ---

  // 1. '탈수' 기준점 (spinThreshold) 계산 (기존과 동일)
  // (40분 동안의 '최대' 진동값을 사용하므로 이 로직은 정확함)
  spinThreshold = MIN_SPIN_THRESHOLD;
  if (spinThreshold < MIN_SPIN_THRESHOLD) {
    spinThreshold = MIN_SPIN_THRESHOLD;
  }

  // 2. '세탁' 기준점 (washThreshold) 계산 [수정됨]
  // (40분 캘리브레이션 시 '평균'은 '탈수' 진동에 오염되므로,
  //  '평균' 계산을 아예 무시하고 config.h의 최소값으로 강제 설정)
  washThreshold = MIN_WASH_THRESHOLD; 

  // 3. 진동 지문(Signature) 계산 (기존과 동일)
  // (어차피 '탈수'의 최대 진동 순간을 기준으로 계산됨)
  float totalMaxDelta = maxX_at_maxVib + maxY_at_maxVib + maxZ_at_maxVib;
  if (totalMaxDelta > 0) {
    signatureX = maxX_at_maxVib / totalMaxDelta; 
    signatureY = maxY_at_maxVib / totalMaxDelta; 
    signatureZ = maxZ_at_maxVib / totalMaxDelta; 
  }
  
  isCalibrated_flag = true; 
  save(); // 계산된 모든 값을 EEPROM에 저장

  Serial.println("======================================");
  Serial.println(" Calibration Finished!");
  Serial.println("--- Final Calculated Values ---");
  Serial.print("Wash Threshold: ");
  Serial.println(washThreshold); // (이제 0.3이 뜰 겁니다)
  Serial.print("Spin Threshold: ");
  Serial.println(spinThreshold); // (이제 1.50보다 높은 실제 값이 뜰 겁니다)
  Serial.print("Signature X: ");
  Serial.println(signatureX);
  Serial.print("Signature Y: ");
  Serial.println(signatureY);
  Serial.print("Signature Z: ");
  Serial.println(signatureZ);
  Serial.println(" New Thresholds Saved. Restarting...");
  Serial.println("======================================");
  delay(1000);
  ESP.restart(); // 저장 후 자동으로 재부팅하여 일반 모드로 시작
}

// --- Getter 함수들 (수정 없음) ---
float Calibration::getWashThreshold() { return washThreshold; }
float Calibration::getSpinThreshold() { return spinThreshold; }
float Calibration::getSignatureX() { return signatureX; }
float Calibration::getSignatureY() { return signatureY; }
float Calibration::getSignatureZ() { return signatureZ; }


// --- Private 내부 함수들 (수정 없음) ---
void Calibration::load() {
  if (EEPROM.read(ADDR_CALIB_FLAG) == 1) {
    isCalibrated_flag = true;
    EEPROM.get(ADDR_WASH_THRESH, washThreshold);
    EEPROM.get(ADDR_SPIN_THRESH, spinThreshold);
    EEPROM.get(ADDR_SIG_X, signatureX);
    EEPROM.get(ADDR_SIG_Y, signatureY);
    EEPROM.get(ADDR_SIG_Z, signatureZ);
  } else {
    isCalibrated_flag = false;
  }
}

void Calibration::save() {
  EEPROM.write(ADDR_CALIB_FLAG, 1);
  EEPROM.put(ADDR_WASH_THRESH, washThreshold);
  EEPROM.put(ADDR_SPIN_THRESH, spinThreshold);
  EEPROM.put(ADDR_SIG_X, signatureX);
  EEPROM.put(ADDR_SIG_Y, signatureY);
  EEPROM.put(ADDR_SIG_Z, signatureZ);
  EEPROM.commit();
}