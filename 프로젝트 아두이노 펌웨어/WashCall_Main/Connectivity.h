#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class Connectivity {
  public:
    Connectivity();
    // [수정됨] 2개의 URL을 받도록 변경
    void begin(const char* ssid, const char* pass, String update_url, String raw_url, int id);

    // [유지됨] StateManager가 사용할 기존 함수 (timeElapsed는 제거된 버전)
    void sendReport(String state, bool isFinished, float washAvg, float washMax, float spinMax);

    // [추가됨] 1초 스트리밍용 새 함수
    void sendRawData(long timestamp, float magnitude, float deltaX, float deltaY, float deltaZ);

  private:
    String api_url_update; // [수정됨] /update URL 저장용
    String api_url_raw;    // [추가됨] /raw_data URL 저장용
    int machine_id;
};

#endif
