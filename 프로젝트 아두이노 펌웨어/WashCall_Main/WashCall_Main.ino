#include "config.h"
#include "VibrationSensor.h"
#include "Calibration.h"
#include "StateManager.h"
#include "Connectivity.h"

// --- config.h에서 extern으로 선언한 변수들의 실제 값을 여기서 정의 ---
const char* WIFI_SSID = "iPhone24"; // 실제 사용할 Wi-Fi 이름으로 변경
const char* WIFI_PASSWORD = "12345678"; // 실제 사용할 Wi-Fi 비밀번호로 변경
String FASTAPI_URL = "http://172.20.10.6:8000/update"; // 단일 API 경로 포함
int MACHINE_ID = 2;
extern const char* SECRET_KEY;
const char* SECRET_KEY = "secret_key_123"; // FastAPI 서버와 약속한 비밀 키
// ----------------------------------------------------------------

VibrationSensor sensor;
Calibration calibration;
StateManager stateManager;
Connectivity connectivity;

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Wash&Call System Initializing ---");

  sensor.begin();
  calibration.begin();
  stateManager.begin(calibration, sensor, connectivity);

  // 캘리브레이션이 완료된 경우에만 Wi-Fi 연결 시도
  if (!calibration.needsCalibration()) {
    // connectivity.begin에는 전체 URL(FASTAPI_URL)이 전달됩니다.
    connectivity.begin(WIFI_SSID, WIFI_PASSWORD, FASTAPI_URL, MACHINE_ID);
  }

  Serial.println("--- System Ready ---");
}

void loop() {
  stateManager.update();
  delay(1000); // 1초마다 상태 업데이트 확인
}