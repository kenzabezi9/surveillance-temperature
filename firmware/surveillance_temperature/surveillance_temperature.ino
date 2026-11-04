#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"

// Définition des broches
#define DHTPIN 2
#define DHTTYPE DHT22
#define RED_LED 8
#define BLUE_LED 9
#define BUZZER 10

// Initialisation des objets
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adresse I2C = 0x27 (changer si nécessaire)

void setup() {
  // Initialisation des composants
  dht.begin();
  lcd.init();
  lcd.backlight();
  
  pinMode(RED_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Systeme Demarre...");
  delay(2000);
  lcd.clear();
}

void loop() {
  float temperature = dht.readTemperature();

  // Vérifie si le capteur répond
  if (isnan(temperature)) {
    lcd.setCursor(0, 0);
    lcd.print("Erreur capteur!");
    return;
  }

  // Affichage
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C   "); // petits espaces pour effacer les anciennes valeurs

  // Gestion LED et buzzer
  if (temperature > 30.0) {
    digitalWrite(RED_LED, HIGH);
    digitalWrite(BLUE_LED, LOW);
    digitalWrite(BUZZER, HIGH);  // Alerte sonore
  } else {
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, HIGH);
    digitalWrite(BUZZER, LOW);   // Silence
  }

  delay(2000);  // Pause de 2 secondes
}
