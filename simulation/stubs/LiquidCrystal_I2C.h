#pragma once
#include <stdio.h>
#include <string>
extern std::string ecran[2];
extern int caracteresHorsEcran;
struct LiquidCrystal_I2C {
  int ligne = 0;
  LiquidCrystal_I2C(int, int, int) {}
  void init() {}
  void backlight() {}
  void clear() { ecran[0].clear(); ecran[1].clear(); }
  void setCursor(int, int l) { ligne = l; }
  void ecrire(const std::string& t) {
    ecran[ligne] += t;
    if (ecran[ligne].size() > 16) caracteresHorsEcran += ecran[ligne].size() - 16;
  }
  void print(const char* t) { ecrire(t); }
  void print(float v, int decimales) {
    char b[24];
    snprintf(b, sizeof b, "%.*f", decimales, v);
    ecrire(b);
  }
};
