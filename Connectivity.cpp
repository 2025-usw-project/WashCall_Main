#include "Connectivity.h"
#include "config.h"

Connectivity::Connectivity() {
  // 생성자는 비워둠
}

// Wi-Fi 연결 및 기본 정보 설정
void Connectivity::begin(const char* ssid, const char* pass, String url, String id) {
  this->firebase_url = url;
  this->machine_id = id;

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");
}

// 현재 상태를 서버로 전송하는 함수
void Connectivity::sendState(String state) {
  // 1. Wi-Fi 연결 상태 확인
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // 2. 보낼 데이터 생성 (JSON)
    StaticJsonDocument<200> jsonDoc;
    jsonDoc["machine_id"] = machine_id;
    jsonDoc["status"] = state;
    // (나중에 배터리 잔량 등 추가 정보 포함 가능)
    
    String jsonPayload;
    serializeJson(jsonDoc, jsonPayload);

    // 3. 서버로 HTTP POST 요청 보내기
    http.begin(firebase_url);
    http.addHeader("Content-Type", "application/json");
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
    Serial.println("WiFi Disconnected. Cannot send data.");
  }
}