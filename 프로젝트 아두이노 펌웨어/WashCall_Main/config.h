#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h>

// =================================================================
// 1. 네트워크 설정 (Network Configuration)
// =================================================================
// 변수의 '실제 내용'은 Main.ino 파일에 있고, 여기서는 '이름'만 알려준다고 선언 (extern)
extern const char* WIFI_SSID;
extern const char* WIFI_PASSWORD;
extern String FASTAPI_URL;
extern int MACHINE_ID;
extern const char* SECRET_KEY;

// =================================================================
// 2. 하드웨어 핀 설정 (Hardware Pin Configuration)
// =================================================================
#define MPU_SDA_PIN 21
#define MPU_SCL_PIN 22

// =================================================================
// 3. 알고리즘 상수 설정 (Algorithm Constants)
// =================================================================
const long CALIBRATION_DURATION_MS = 10000; 
const long FINISHED_TIMEOUT_MS = 15000;
const float PATTERN_MATCH_TOLERANCE = 0.35; 
const float MIN_WASH_THRESHOLD = 0.3;
const float MIN_SPIN_THRESHOLD = 1.5;

// =================================================================
// 4. EEPROM 주소 설정 (EEPROM Address Map)
// =================================================================
#define EEPROM_SIZE 30
#define ADDR_CALIB_FLAG 0
#define ADDR_WASH_THRESH 1
#define ADDR_SPIN_THRESH 5
#define ADDR_SIG_X 9
#define ADDR_SIG_Y 13
#define ADDR_SIG_Z 17

#endif