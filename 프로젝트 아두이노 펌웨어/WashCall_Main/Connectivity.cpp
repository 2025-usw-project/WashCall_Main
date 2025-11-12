#include "Connectivity.h"
#include "config.h"
#include "time.h"

Connectivity::Connectivity() {
  // 생성자는 비워둠
}

// [수정됨] 2개의 URL을 받도록 변경
void Connectivity::begin(const char* ssid, const char* pass, String update_url, String raw_url, int id) {
  this->api_url_update = update_url; // (.../update)
  this->api_url_raw = raw_url;       // (.../raw_data)
  this->machine_id = id;

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  Serial.println("Synchronizing time with NTP server...");
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" Time synchronized!");
}

// [유지됨] StateManager가 사용할 기존 함수
void Connectivity::sendReport(String state, bool isFinished, float washAvg, float washMax, float spinMax) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(api_url_update); // [수정됨] 상태 보고용 URL
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<512> jsonDoc;

    jsonDoc["machine_id"] = machine_id;
    jsonDoc["secret_key"] = SECRET_KEY;
    jsonDoc["status"] = state;
    jsonDoc["machine_type"] = "washer";
    jsonDoc["timestamp"] = time(nullptr);
    jsonDoc["battery"] = 100;

    if (isFinished) {
      jsonDoc["wash_avg_magnitude"] = washAvg;
      jsonDoc["wash_max_magnitude"] = washMax;
      jsonDoc["spin_max_magnitude"] = spinMax;
    }

    String jsonPayload;
    serializeJson(jsonDoc, jsonPayload);

    Serial.println("Sending Report Data to Server:");
    Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending POST (Report): ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected. Cannot send report data.");
  }
}

// [추가됨] 1초 스트리밍 데이터 전송 함수
void Connectivity::sendRawData(long timestamp, float magnitude, float deltaX, float deltaY, float deltaZ) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(api_url_raw); // [수정됨] 원본 데이터 전송용 URL
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<256> jsonDoc;

    jsonDoc["machine_id"] = machine_id;
    jsonDoc["timestamp"] = timestamp;
    jsonDoc["magnitude"] = magnitude;
    jsonDoc["deltaX"] = deltaX;
    jsonDoc["deltaY"] = deltaY;
    jsonDoc["deltaZ"] = deltaZ;

    String jsonPayload;
    serializeJson(jsonDoc, jsonPayload);

    // 1초마다 로그가 너무 많이 뜨므로 주석 처리
    // Serial.println("Sending Raw Data:");
    // Serial.println(jsonPayload);

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode <= 0) {
      Serial.print("Error sending POST (Raw): ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected. Cannot send raw data.");
  }
}
