#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

// Définition du taux de rafraîchissement de l'affichage LVGL
ThreadLvgl threadLvgl(30);

// Prototype des fonctions
void lv_affiche_text(float valSensor);

// Déclaration E/S
AnalogIn sensor(A0);       // Capteur de luminosité sur A0
DigitalOut buzzer(D2);     // Buzzer sur D2

int main() {
    
    buzzer = 1; // Active le buzzer (bruit)
    buzzer = 0; // Désactive le buzzer (pas de bruit)
    
    while (1) {
        // Lecture de la valeur du capteur
        float valSensor = sensor.read(); // Lire la valeur du capteur (0.0 à 1.0)
        
        // Utilisation du mutex pour les affichages LVGL
        threadLvgl.lock();
        lv_affiche_text(valSensor); // Affichage de la valeur
        threadLvgl.unlock();
        
        // Tempo de 100 ms entre les lectures
        ThisThread::sleep_for(100ms);
    }
}

// Fonction pour afficher sur un écran LVGL
void lv_affiche_text(float valSensor) {
    // Conversion de la valeur du capteur en chaîne de caractères
    char buf[32];
    sprintf(buf, "Luminosite : %2f", valSensor);

    // Create a style for the shadow
    static lv_style_t style_shadow;
    lv_style_init(&style_shadow);
    lv_style_set_text_opa(&style_shadow, LV_OPA_30);
    lv_style_set_text_color(&style_shadow, lv_color_black());

    //Create a label for the shadow first (it's in the background)
    static lv_obj_t *shadow_label = nullptr;
    if (shadow_label == nullptr) {
        shadow_label = lv_label_create(lv_scr_act());
        lv_obj_add_style(shadow_label, &style_shadow, 0);
    }

    //Create the main label
    static lv_obj_t *main_label = nullptr;
    if (main_label == nullptr) {
        main_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(main_label, buf); // Affichage de la valeur du capteur

    //Set the same text for the shadow label
    lv_label_set_text(shadow_label, lv_label_get_text(main_label));

    //Position the main label
    lv_obj_align(main_label, LV_ALIGN_CENTER, 0, 0);

    //Shift the second label down and to the right by 2 pixel
    lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);
}

//idee :
//  Pour le clavier numerique utiliser text auto-formatting
//  Pour affichage heure actu utiliser a clock from a meter
//  Pour affichage luminosite utiliser text shadow
