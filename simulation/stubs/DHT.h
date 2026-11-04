#pragma once
#define DHT22 22
float capteurTemperature();
float capteurHumidite();
struct DHT {
  DHT(int, int) {}
  void begin() {}
  float readTemperature() { return capteurTemperature(); }
  float readHumidity() { return capteurHumidite(); }
};
