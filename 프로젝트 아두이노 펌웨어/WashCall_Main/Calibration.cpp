#include <EEPROM.h>
#include "config.h"
#include "Calibration.h"

// 생성자
Calibration::Calibration() {
  isCalibrated_flag = false;
  washThreshold = 0.4;  // 초기 기본값
  spinThreshold = 1.8; // 초기 기본값
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
  double totalVibration = 0; // 합산 값은 매우 커질 수 있으므로 double 사용
  long vibrationCount = 0;

  unsigned long startTime = millis();
  while (millis() - startTime < CALIBRATION_DURATION_MS) {
    VibrationData data = sensor.getVibration();

    // 1초마다 현재 상태 출력
    Serial.print("Calibrating... Current Vibration: ");
    Serial.println(data.magnitude);

    if (data.magnitude > 0.25) { // 의미 있는 진동만 데이터에 포함
      if (data.magnitude > maxVibration) {
        maxVibration = data.magnitude; // 과정 중에서 제일 큰 진동 값 갱신해주기
        // 최대 진동 순간의 X,Y,Z 델타 값 저장
        maxX_at_maxVib = data.deltaX; // 최대 진동 당시의 X축
        maxY_at_maxVib = data.deltaY; // 최대 진동 당시의 Y축
        maxZ_at_maxVib = data.deltaZ; // 최대 진동 당시의 Z축
      }
      totalVibration += data.magnitude; // 총 진동 값
      vibrationCount++; // 진동 횟수
    }
    delay(1000);
  }

  // 기준점 및 '진동 지문' 최종 계산
  spinThreshold = maxVibration * 0.6; // 탈수 기준점 (최대 진동값에서 살짝 여유를 둔 0.7을 곱한 값)
  
  // 0으로 나누는 오류 방지
  if (vibrationCount > 0) {
    float avgVibration = totalVibration / vibrationCount; // 평균 진동 값 (전체 진동에서 횟수로 나누기)
    washThreshold = avgVibration * 0.7; // 세탁 기준점 (세탁 중에도 갑자기 진동이 튈수도 있어서 1.5 곱해주기)
  }

  // 너무 낮은 값이 설정되는 것 방지
  if (washThreshold < MIN_WASH_THRESHOLD) washThreshold = MIN_WASH_THRESHOLD;
  if (spinThreshold < MIN_SPIN_THRESHOLD) spinThreshold = MIN_SPIN_THRESHOLD;

  // 진동 지문(Signature) 계산
  float totalMaxDelta = maxX_at_maxVib + maxY_at_maxVib + maxZ_at_maxVib;
  if (totalMaxDelta > 0) {
    signatureX = maxX_at_maxVib / totalMaxDelta; // X축 진동 지문 기준점
    signatureY = maxY_at_maxVib / totalMaxDelta; // Y축 진동 지문 기준점
    signatureZ = maxZ_at_maxVib / totalMaxDelta; // Z축 진동 지문 기준점
  }
  
  isCalibrated_flag = true; // 캘리브레이션 완료 플래그 설정
  save(); // 계산된 모든 값을 EEPROM에 저장

  Serial.println("======================================");
  Serial.println(" Calibration Finished!");
  Serial.println("--- Final Calculated Values ---");
  Serial.print("Wash Threshold: ");
  Serial.println(washThreshold);
  Serial.print("Spin Threshold: ");
  Serial.println(spinThreshold);
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

// --- Getter 함수들 ---
float Calibration::getWashThreshold() { return washThreshold; }
float Calibration::getSpinThreshold() { return spinThreshold; }
float Calibration::getSignatureX() { return signatureX; }
float Calibration::getSignatureY() { return signatureY; }
float Calibration::getSignatureZ() { return signatureZ; }


// --- Private 내부 함수들 ---
void Calibration::load() {
  if (EEPROM.read(ADDR_CALIB_FLAG) == 1) { // 켈리브레이션 완료 여부가 들어가있는 주소
    isCalibrated_flag = true;
    EEPROM.get(ADDR_WASH_THRESH, washThreshold); // ROM에서 세탁 기준점이 들어가있는 주소
    EEPROM.get(ADDR_SPIN_THRESH, spinThreshold); // ROM에서 탈수 기준점이 들어가있는 주소
    EEPROM.get(ADDR_SIG_X, signatureX); // ROM에서 X축 기준점이 들어가있는 주소
    EEPROM.get(ADDR_SIG_Y, signatureY); // ROM에서 Y축 기준점이 들어가있는 주소
    EEPROM.get(ADDR_SIG_Z, signatureZ); // ROM에서 Z축 기준점이 들어가있는 주소
  } else {
    isCalibrated_flag = false;
  }
}

void Calibration::save() {
  EEPROM.write(ADDR_CALIB_FLAG, 1); // 저장된 파일이 있는 상태라는 걸 표시
  EEPROM.put(ADDR_WASH_THRESH, washThreshold); // ROM에 세탁 기준점을 저장
  EEPROM.put(ADDR_SPIN_THRESH, spinThreshold); // ROM에 탈수 기준점을 저장
  EEPROM.put(ADDR_SIG_X, signatureX); // ROM에 X축 기준점을 저장
  EEPROM.put(ADDR_SIG_Y, signatureY); // ROM에 Y축 기준점을 저장
  EEPROM.put(ADDR_SIG_Z, signatureZ); // ROM에 Z축 기준점을 저장
  EEPROM.commit(); // 저장 완료 확정하기
}