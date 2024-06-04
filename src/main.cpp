#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

AnalogIn sensor(A0); //pin 9 de l'arduino pour capteur
DigitalOut buzzer(D2); //pin 17 de l'arduino pour buzzer

int main() {

    buzzer=1; //buzzer à 1 (bruit)
    buzzer=0; //buzzer à 0 (pas de bruit)
    threadLvgl.lock(); //mutex a utilise pour les affichages


    while (1) {
        // put your main code here, to run repeatedly:
        //ThisThread::sleep_for(10ms);//tempo 10ms
        lv_demo_widgets(); //creation des affichages
        
        threadLvgl.unlock(); //mutex a utilise pour les affichages
    }
}

//idee :
//  Pour le clavier numerique utiliser text auto-formatting
//  Pour affichage heure actu utiliser a clock from a meter
//  Pour affichage luminosite utiliser text shadow
