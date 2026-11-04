// Reconstitution avec ventilateur, validée en simulation.
// Le ventilateur (D6) doit passer par un transistor, jamais directement par la broche.
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#include "controle.h"

const int BROCHE_DHT = 2;
const int LED_ROUGE = 8;
const int LED_BLEUE = 9;
const int BUZZER = 10;
const int VENTILATEUR = 6;
const unsigned long PERIODE_MS = 2000;  // le DHT22 ne se lit pas plus vite

DHT dht(BROCHE_DHT, DHT22);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Etat etat;
unsigned long derniereLecture = 0;

void appliquer(const Sorties& s) {
  digitalWrite(LED_ROUGE, s.ledRouge ? HIGH : LOW);
  digitalWrite(LED_BLEUE, s.ledBleue ? HIGH : LOW);
  digitalWrite(BUZZER, s.buzzer ? HIGH : LOW);
  digitalWrite(VENTILATEUR, s.ventilateur ? HIGH : LOW);
}

void afficher(const Sorties& s, float temperature, float humidite, bool valide) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if (valide) {
    lcd.print("T:");
    lcd.print(temperature, 1);
    lcd.print("C H:");
    lcd.print(humidite, 0);
    lcd.print("%");
  } else {
    lcd.print("Erreur capteur");
  }
  lcd.setCursor(0, 1);
  if (s.panneCapteur) {
    lcd.print("Mode securite");
  } else {
    lcd.print(etat.alerte ? "ALERTE " : "Seuil ");
    lcd.print(SEUIL_ALERTE, 1);
    lcd.print("C");
  }
}

void setup() {
  pinMode(LED_ROUGE, OUTPUT);
  pinMode(LED_BLEUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(VENTILATEUR, OUTPUT);
  dht.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Demarrage...");
  delay(PERIODE_MS);
}

void loop() {
  if (millis() - derniereLecture < PERIODE_MS) return;
  derniereLecture = millis();

  float temperature = dht.readTemperature();
  float humidite = dht.readHumidity();
  bool valide = !isnan(temperature) && !isnan(humidite);

  Sorties s = mettreAJour(etat, temperature, valide);
  appliquer(s);
  afficher(s, temperature, humidite, valide);
}
