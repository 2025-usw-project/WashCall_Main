#include "config.h"
#include "VibrationSensor.h"
#include "Calibration.h"
#include "StateManager.h"
#include "Connectivity.h"

// --- config.h에서 extern으로 선언한 변수들의 실제 값을 여기서 정의 ---
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
String FIREBASE_URL = "YOUR_FIREBASE_URL";
String MACHINE_ID = "washer_01";
// ----------------------------------------------------------------

// 모든 모듈(부품) 생성
VibrationSensor sensor;
Calibration calibration;
StateManager stateManager;
Connectivity connectivity;

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Wash&Call System Initializing ---");

  // 1. 센서 초기화
  sensor.begin();
  // 2. 캘리브레이션 모듈 초기화 (EEPROM 로드)
  calibration.begin();
  // 3. 통신 모듈 초기화 (Wi-Fi 연결)
  // (캘리브레이션이 완료된 경우에만 Wi-Fi에 연결)
  if (!calibration.needsCalibration()) {
    connectivity.begin(WIFI_SSID, WIFI_PASSWORD, FIREBASE_URL, MACHINE_ID);
  }
  // 4. 상태 관리자 초기화 (모든 부품 연결)
  stateManager.begin(calibration, sensor, connectivity);
  
  Serial.println("--- System Ready ---");
}

void loop() {
  // 1초마다 상태 관리자를 호출하여 모든 작업을 처리
  stateManager.update();
  delay(1000);
}