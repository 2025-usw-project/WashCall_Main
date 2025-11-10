#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class Connectivity {
  public:
    Connectivity();
    void begin(const char* ssid, const char* pass, String url, int id);

    // 단일 리포트 전송 함수
    // isFinished: 현재 상태가 FINISHED인지 여부
    // washAvg ~ extCnt: FINISHED 상태일 때만 유효한 운영 데이터 값들
    void sendReport(String state, bool isFinished, float washAvg, float washMax, float spinMax, long timeElapsed);

  private:
    String api_url; // 변수 이름 변경 (firebase_url -> api_url, 전체 경로 포함)
    int machine_id;
};

#endif