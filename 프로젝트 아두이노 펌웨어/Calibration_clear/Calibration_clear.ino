#include <EEPROM.h>

// config.h에 있던 EEPROM 관련 설정들을 그대로 가져옵니다.
#define EEPROM_SIZE 30
#define ADDR_CALIB_FLAG 0
#define ADDR_WASH_THRESH 1
#define ADDR_SPIN_THRESH 5
#define ADDR_SIG_X 9
#define ADDR_SIG_Y 13
#define ADDR_SIG_Z 17


void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Starting EEPROM Clear Sketch ---");

  // EEPROM 시작
  EEPROM.begin(EEPROM_SIZE);

  // 1. 캘리브레이션 완료 플래그를 '미완료(0)'로 설정
  EEPROM.write(ADDR_CALIB_FLAG, 0);

  // 2. 저장되어 있던 모든 기준점 값들을 0으로 덮어쓰기
  EEPROM.put(ADDR_WASH_THRESH, 0.0f);
  EEPROM.put(ADDR_SPIN_THRESH, 0.0f);
  EEPROM.put(ADDR_SIG_X, 0.0f);
  EEPROM.put(ADDR_SIG_Y, 0.0f);
  EEPROM.put(ADDR_SIG_Z, 0.0f);

  // 3. 변경사항을 EEPROM에 최종 저장
  if (EEPROM.commit()) {
    Serial.println(">>> Successfully cleared all calibration data!");
  } else {
    Serial.println(">>> FAILED to clear EEPROM data.");
  }

  Serial.println("\n--- Task Finished ---");
  Serial.println("You can now re-upload your main WashCall project.");
}

void loop() {
  // 아무것도 하지 않음
  delay(5000);
}
