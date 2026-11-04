// Logique de décision, séparée du matériel pour pouvoir la tester sur PC.
#pragma once

const float SEUIL_ALERTE = 38.0;  // l'alerte démarre au-dessus de cette valeur (°C)
const float SEUIL_RETOUR = 36.0;  // elle s'arrête en dessous de celle-ci (°C)
const int ECHECS_MAX = 3;         // lectures ratées d'affilée avant de déclarer une panne

struct Etat {
  bool alerte = false;
  int echecs = 0;
};

struct Sorties {
  bool ledRouge;
  bool ledBleue;
  bool buzzer;
  bool ventilateur;
  bool panneCapteur;
};

inline Sorties mettreAJour(Etat& etat, float temperature, bool lectureValide) {
  if (lectureValide) {
    etat.echecs = 0;
    if (!etat.alerte && temperature > SEUIL_ALERTE) etat.alerte = true;
    else if (etat.alerte && temperature < SEUIL_RETOUR) etat.alerte = false;
  } else if (etat.echecs < ECHECS_MAX) {
    etat.echecs++;
  }

  // Sans mesure fiable, on se met en sécurité : alerte et ventilateur.
  bool panne = etat.echecs >= ECHECS_MAX;
  bool actif = etat.alerte || panne;
  return Sorties{actif, !actif, actif, actif, panne};
}
