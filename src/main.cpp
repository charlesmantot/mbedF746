#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

int main() {

    const int lightSensor=9;//pin 9 de l'arduino pour capteur
    const int buzzer=17;
    float valeur;
    threadLvgl.lock();//mutex a utilise pour les affichages

    lv_demo_widgets();//creation des affichages

    threadLvgl.unlock();//mutex a utilise pour les affichages

    while (1) {
        // put your main code here, to run repeatedly:
        ThisThread::sleep_for(10ms);
        valeur=analogin_read(0);
        printf("%f",valeur);
    }
}

//idee :
//  Pour le clavier numerique utiliser text auto-formatting
//  Pour affichage heure actu utiliser a clock from a meter
//  Pour affichage luminosite utiliser text shadow
