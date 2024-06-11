#include <mbed.h>
#include <threadLvgl.h>
#include <chrono>
#include <ctime>

#include "demos/lv_demos.h"
#include <cstdio>
#include <stdio.h>

// Définition du taux de rafraîchissement de l'affichage LVGL
ThreadLvgl threadLvgl(30);

// Prototype des fonctions
void lv_affiche_text(float valSensor);
void lv_affiche_time(void);
void clavier(void);
static void orga_saisie(lv_event_t * e);
static void ta_event_cb(lv_event_t * e);
static lv_obj_t * kb;
static lv_obj_t * time_label;

// Déclaration E/S
AnalogIn sensor(A0);       // Capteur de luminosité sur A0
DigitalOut buzzer(D2);     // Buzzer sur D2B
BufferedSerial pc(USBTX, USBRX, 9600); // Pour la transmission des données

int main() {
    float valSensor;
    buzzer = 1; // Active le buzzer (bruit)
    buzzer = 0; // Désactive le buzzer (pas de bruit)

    // Définir l'heure à 09h15
    struct tm t;
    t.tm_year = 2024 - 1900; // Année depuis 1900
    t.tm_mon = 6 - 1;        // Mois [0-11]
    t.tm_mday = 11;          // Jour du mois
    t.tm_hour = 9;           // Heure [0-23]
    t.tm_min = 15;           // Minute [0-59]
    t.tm_sec = 0;            // Seconde [0-59]
    t.tm_isdst = -1;         // L'information sur l'heure d'été est déterminée par le système
    set_time(mktime(&t));    // Régler l'heure du système

    // Initialiser le clavier et l'affichage de l'heure à l'écran
    threadLvgl.lock();
    clavier();
    lv_affiche_time();
    threadLvgl.unlock();
    
    while (1) {
        // Lecture de la valeur du capteur
        valSensor = sensor.read(); // Lire la valeur du capteur (0.0 à 1.0)
        
        threadLvgl.lock(); // Prise du mutex pour les affichages LVGL
        lv_affiche_text(valSensor); // Affichage de la valeur
        lv_affiche_time(); // Affichage de l'heure
        threadLvgl.unlock(); // Relacher le mutex

        if(valSensor < 0.5){ // Si valeur du capteur inférieur à 0 alors le buzzer sonne
            buzzer = 1;
        }
        else { // Sinon il ne sonne pas
            buzzer = 0;
        }
        
        ThisThread::sleep_for(100ms); // Tempo de 100 ms entre les lectures
    }
}

// Fonction pour afficher sur un écran LVGL
void lv_affiche_text(float valSensor) {
    // Conversion de la valeur du capteur en chaîne de caractères
    char buf[32];
    sprintf(buf, "Luminosite : %2.2f", valSensor); // Affiche sur l'écran la valeur du capteur

    // Create a style for the shadow
    static lv_style_t style_shadow;
    lv_style_init(&style_shadow);
    lv_style_set_text_opa(&style_shadow, LV_OPA_30);
    lv_style_set_text_color(&style_shadow, lv_color_black());

    // Create a label for the shadow first (it's in the background)
    static lv_obj_t *shadow_label = nullptr;
    if (shadow_label == nullptr) {
        shadow_label = lv_label_create(lv_scr_act());
        lv_obj_add_style(shadow_label, &style_shadow, 0);
    }

    // Create the main label
    static lv_obj_t *main_label = nullptr;
    if (main_label == nullptr) {
        main_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(main_label, buf); // Affichage de la valeur du capteur

    // Set the same text for the shadow label
    lv_label_set_text(shadow_label, lv_label_get_text(main_label));

    // Position the main label
    lv_obj_align(main_label, LV_ALIGN_TOP_MID, 0, 20); // Aligner en haut au milieu

    // Shift the second label down and to the right by 2 pixel
    lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);
}

// Fonction pour afficher l'heure actuelle sur un écran LVGL
void lv_affiche_time(void) {
    // Obtenir l'heure actuelle
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);

    // Conversion de l'heure en chaîne de caractères
    char buf[32];
    strftime(buf, sizeof(buf), "Heure : %H:%M:%S", local_time); // Au format h : min : sec

    // Création ou mise à jour du label pour l'heure
    if (time_label == nullptr) {
        time_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(time_label, buf);

    // Positionner le label de l'heure en bas au centre
    lv_obj_align(time_label, LV_ALIGN_BOTTOM_MID, 0, -20);
}

void clavier(void) {
    /* Create the text area */
    lv_obj_t * ta = lv_textarea_create(lv_scr_act());
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_textarea_set_accepted_chars(ta, "0123456789:");
    lv_textarea_set_max_length(ta, 5);
    lv_textarea_set_one_line(ta, true);
    lv_textarea_set_text(ta, "");

    /* Create a keyboard */
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2);
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(kb, ta);

    // Position the text area
    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50); // Aligner au milieu en haut avec un offset
}

static void orga_saisie(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    const char * txt = lv_textarea_get_text(ta);
    if(txt[0] >= '0' && txt[0] <= '9' &&
       txt[1] >= '0' && txt[1] <= '9' &&
       txt[2] != ':') {
        lv_textarea_set_cursor_pos(ta, 2);
        lv_textarea_add_char(ta, ':');
    }
}

static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        orga_saisie(e);
    } else if(code == LV_EVENT_READY) {
        /* Enter was pressed. */
        const char * txt = lv_textarea_get_text(ta);
        printf("Text: %s\n", txt);
        lv_textarea_set_text(ta, "");
    }
}

