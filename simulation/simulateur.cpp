// Exécute le vrai firmware sur une carte simulée, dans un modèle thermique simple.
// Les constantes du modèle sont choisies pour l'exemple : elles ne viennent pas de mesures.
#include <stdio.h>
#include <functional>
#include <map>
#include <string>
#include "Arduino.h"

std::string ecran[2];
int caracteresHorsEcran = 0;
static std::map<int, int> broches;
static unsigned long horloge = 0;
static float T = 25;
static double chauffage = 0;
static bool ventilateurEfficace = true;
static bool capteurEnPanne = false;
static std::function<float(unsigned long)> temperatureForcee;

const double T_AMBIANTE = 25, K_NATUREL = 0.002, K_VENTILATEUR = 0.02;

void pinMode(int, int) {}
void digitalWrite(int b, int v) { broches[b] = v; }
unsigned long millis() { return horloge; }
void delay(unsigned long ms) {
  for (unsigned long i = 0; i < ms; i += 100) {
    horloge += 100;
    if (temperatureForcee) { T = temperatureForcee(horloge); continue; }
    double k = K_NATUREL + ((broches[6] && ventilateurEfficace) ? K_VENTILATEUR : 0);
    T += 0.1 * (chauffage - k * (T - T_AMBIANTE));
  }
}
float capteurTemperature() { return capteurEnPanne ? NAN : T; }
float capteurHumidite() { return capteurEnPanne ? NAN : 45; }

#include "../firmware/version_ventilateur/version_ventilateur.ino"

static int echecs = 0, verifications = 0;
static void verifie(bool ok, const char* quoi) {
  verifications++;
  printf("  [%s] %s\n", ok ? "OK" : "ECHEC", quoi);
  if (!ok) echecs++;
}
static void depart(float t0, double puissance) {
  broches.clear(); ecran[0].clear(); ecran[1].clear();
  horloge = 0; T = t0; chauffage = puissance;
  ventilateurEfficace = true; capteurEnPanne = false; temperatureForcee = nullptr;
  etat = Etat(); derniereLecture = 0;
  setup();
}
static void avance(unsigned long ms, const std::function<void()>& apres = [] {}) {
  for (unsigned long fin = horloge + ms; horloge < fin;) { loop(); delay(100); apres(); }
  loop();
}
static bool actif() { return broches[LED_ROUGE] && broches[BUZZER] && broches[VENTILATEUR] && !broches[LED_BLEUE]; }
static bool repos() { return !broches[LED_ROUGE] && !broches[BUZZER] && !broches[VENTILATEUR] && broches[LED_BLEUE]; }

int main() {
  puts("Seuils de la logique");
  { Etat e;
    verifie(!mettreAJour(e, 38.0, true).ventilateur, "38,0 C : pas encore d'alerte");
    verifie(mettreAJour(e, 38.1, true).ventilateur, "38,1 C : alerte");
    verifie(mettreAJour(e, 36.0, true).ventilateur, "36,0 C : l'alerte continue");
    verifie(!mettreAJour(e, 35.9, true).ventilateur, "35,9 C : l'alerte s'arrete"); }

  puts("Temperature normale");
  depart(25, 0);
  bool toujoursRepos = true;
  avance(600000, [&] { if (horloge > 4100 && !repos()) toujoursRepos = false; });
  verifie(toujoursRepos, "10 minutes a 25 C : LED bleue seule, ventilateur arrete");
  verifie(ecran[0] == "T:25.0C H:45%" && ecran[1] == "Seuil 38.0C", "l'ecran affiche temperature, humidite et seuil");

  puts("Echauffement avec ventilateur efficace");
  depart(25, 0.06);
  FILE* csv = fopen("build/trace.csv", "w");
  if (csv) fprintf(csv, "temps_s,temperature_c,ventilateur\n");
  float maxi = 0; long tDepassement = -1, tDeclenchement = -1; int demarrages = 0, avant = 0;
  avance(1200000, [&] {
    if (T > maxi) maxi = T;
    if (tDepassement < 0 && T > SEUIL_ALERTE) tDepassement = horloge;
    if (tDeclenchement < 0 && broches[VENTILATEUR]) tDeclenchement = horloge;
    if (broches[VENTILATEUR] && !avant) demarrages++;
    avant = broches[VENTILATEUR];
    if (csv && horloge % 5000 == 0) fprintf(csv, "%lu,%.2f,%d\n", horloge / 1000, T, avant);
  });
  if (csv) fclose(csv);
  verifie(tDepassement > 0 && tDeclenchement >= tDepassement && tDeclenchement - tDepassement <= 2200, "le ventilateur demarre moins de 2,2 s apres le depassement");
  verifie(maxi < 39, "la temperature reste sous 39 C");
  verifie(demarrages >= 2, "le ventilateur s'arrete et redemarre selon la temperature");
  printf("  depassement a %.1f s, ventilateur a %.1f s, maximum %.2f C, %d demarrages\n", tDepassement / 1000.0, tDeclenchement / 1000.0, maxi, demarrages);

  puts("Mesure qui oscille autour du seuil");
  depart(38, 0);
  temperatureForcee = [](unsigned long ms) { return 38.0f + ((ms / 2000) % 2 ? 0.3f : -0.3f); };
  int bascules = 0; avant = 0; bool premier = true;
  avance(300000, [&] { if (!premier && broches[VENTILATEUR] != avant) bascules++; avant = broches[VENTILATEUR]; premier = false; });
  verifie(bascules <= 1, "37,7/38,3 C pendant 5 minutes : pas de basculement en boucle");

  puts("Panne du capteur");
  depart(25, 0);
  avance(30000);
  capteurEnPanne = true;
  avance(2000);
  verifie(repos(), "une seule lecture ratee ne declenche rien");
  avance(6000);
  verifie(actif() && ecran[0] == "Erreur capteur" && ecran[1] == "Mode securite", "apres 3 lectures ratees : mode securite");
  capteurEnPanne = false;
  avance(4000);
  verifie(repos(), "le capteur revient : retour a la normale");

  puts("Ventilateur en panne");
  depart(25, 0.06);
  ventilateurEfficace = false;
  avance(1200000);
  verifie(T > 38 && actif(), "l'alerte reste active pendant que la temperature monte");
  printf("  temperature apres 20 minutes : %.1f C\n", T);

  verifie(caracteresHorsEcran == 0, "aucun texte ne depasse l'ecran de 16 caracteres");
  printf("\n%d verifications, %d echec(s)\n", verifications, echecs);
  return echecs ? 1 : 0;
}
