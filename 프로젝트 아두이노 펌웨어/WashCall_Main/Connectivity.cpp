#include "Connectivity.h"
#include "config.h" // SECRET_KEY 사용을 위해 포함

Connectivity::Connectivity() {
  // 생성자는 비워둠
}

// Wi-Fi 연결 및 API URL 설정
void Connectivity::begin(const char* ssid, const char* pass, String url, int id) {
  // ino 파일에서 받은 전체 API URL(/update 포함)을 저장
  this->api_url = url;
  this->machine_id = id;

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

//  단일 리포트 전송 함수 구현 
void Connectivity::sendReport(String state, bool isFinished, float washAvg, float washMax, float spinMax, long timeElapsed) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // ino 파일에 정의된 전체 API 경로를 사용
    http.begin(api_url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<512> jsonDoc;

    // --- 항상 포함되는 기본 필드들 ---
    jsonDoc["machine_id"] = machine_id;
    jsonDoc["secret_key"] = SECRET_KEY;
    jsonDoc["status"] = state;
    jsonDoc["machine_type"] = "washer";
    jsonDoc["timestamp"] = 0; // 실제 시간으로 변경 필요 (NTP 동기화)
    jsonDoc["battery"] = 100; // 실제 배터리 값으로 변경 필요
    jsonDoc["time_elapsed"] = timeElapsed; // 경과 시간

    // --- FINISHED 상태일 때만 포함되는 사이클 요약 필드들 ---
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
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected. Cannot send report data.");
  }
}