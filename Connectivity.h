#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> // JSON 데이터를 만들기 위한 라이브러리

class Connectivity {
  public:
    Connectivity(); // 생성자 추가
    void begin(const char* ssid, const char* pass, String url, String id);
    void sendState(String state);

  private:
    String firebase_url;
    String machine_id;
};

#endif