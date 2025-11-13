#include "config.h"
#include "VibrationSensor.h"
#include "Calibration.h"
#include "StateManager.h"
#include "Connectivity.h"

// --- config.h에서 extern으로 선언한 변수들의 실제 값을 여기서 정의 ---
const char* WIFI_SSID = "iPhone24";
const char* WIFI_PASSWORD = "12345678";
String FASTAPI_URL = "http://server.washcall.space/update";
String RAW_DATA_URL = "http://server.washcall.space/raw_data"; // [1초 원본 값 경로]
int MACHINE_ID = 1;
extern const char* SECRET_KEY;
const char* SECRET_KEY = "secret_key_123";

// [NTP 변수 정의는 기존과 동일]
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 9 * 3600;
const int DAYLIGHT_OFFSET_SEC = 0;
// ----------------------------------------------------------------

VibrationSensor sensor;
Calibration calibration;
StateManager stateManager;
Connectivity connectivity;

void setup() {
  Serial.begin(115200);
  Serial.println("\n--- Wash&Call System Initializing (Dual Mode) ---"); // [수정됨]

  sensor.begin();
  calibration.begin();
  stateManager.begin(calibration, sensor, connectivity);

  if (!calibration.needsCalibration()) {
    // [수정됨] 2개의 URL을 전달
    connectivity.begin(WIFI_SSID, WIFI_PASSWORD, FASTAPI_URL, RAW_DATA_URL, MACHINE_ID);
  }

  Serial.println("--- System Ready ---");
}

// [수정됨] loop() 로직 전체 변경
void loop() {
  // 1. 센서 값은 loop에서 단 한 번만 읽습니다.
  VibrationData data = sensor.getVibration();

  // 2. StateManager는 이 데이터를 받아 '상태'를 판단합니다.
  stateManager.update(data); // "data"를 인자로 전달
  
  // 3. Connectivity는 이 데이터를 받아 1초마다 스트리밍
  //    (단, 캘리브레이션 중이 아닐 때만 전송)
  if (!calibration.needsCalibration()) {
    long now = time(nullptr);
    connectivity.sendRawData(now, data.magnitude, data.deltaX, data.deltaY, data.deltaZ);
  }
  
  delay(1000); // 1초마다 상태 업데이트 확인
}