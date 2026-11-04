// Fausse carte Arduino pour exécuter le firmware sur PC.
#pragma once
#include <math.h>
#include <stdint.h>
#define HIGH 1
#define LOW 0
#define OUTPUT 1
void pinMode(int broche, int mode);
void digitalWrite(int broche, int valeur);
void delay(unsigned long ms);
unsigned long millis();
