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
BufferedSerial pc(USBTX, USBRX, 9600);

int main() {
    float valSensor;
    buzzer = 1; // Active le buzzer (bruit)
    buzzer = 0; // Désactive le buzzer (pas de bruit)

    // Configurer le fuseau horaire de Paris (UTC+1 en hiver, UTC+2 en été)
    setenv("TZ", "Europe/Paris", 1);
    tzset();

    // Initialiser le clavier et l'affichage de l'heure à l'écran
    threadLvgl.lock();
    clavier();
    lv_affiche_time();
    threadLvgl.unlock();
    
    while (1) {
        // Lecture de la valeur du capteur
        valSensor = sensor.read(); // Lire la valeur du capteur (0.0 à 1.0)
        
        // Utilisation du mutex pour les affichages LVGL
        threadLvgl.lock();
        lv_affiche_text(valSensor); // Affichage de la valeur
        lv_affiche_time(); // Affichage de l'heure
        
        threadLvgl.unlock();
        if(valSensor < 0.5){ // Si valeur du capteur inférieur à 0 alors le buzzer sonne
            buzzer = 1;
        }
        else { // Sinon il ne sonne pas
            buzzer = 0;
        }
        
        // Tempo de 100 ms entre les lectures
        ThisThread::sleep_for(100ms);
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
    lv_obj_align(main_label, LV_ALIGN_TOP_RIGHT, -10, 10); // Affiche en haut à droite de la valeur de luminosite

    // Shift the second label down and to the right by 2 pixel
    lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);
}

// Fonction pour afficher l'heure actuelle sur un écran LVGL
void lv_affiche_time(void) {
    time_t now = time(NULL); // Récup l'heure actuelle
    now += 11.5 * 60; // Ajout 11 minutes 30 sec pour correspondre à l'heure actuelle
    struct tm *local_time = localtime(&now);

    // Conversion de l'heure en chaîne de caractères
    char buf[32];
    strftime(buf, sizeof(buf), "Heure : %H:%M:%S", local_time);

    // Création ou mise à jour du label pour l'heure
    if (time_label == nullptr) {
        time_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(time_label, buf);

    // Position de l'heure en haut à gauche
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 10, 10);
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

/*idee :
  Pour le clavier numerique utiliser text auto-formatting
  Pour affichage heure actu utiliser a clock from a meter
  Pour affichage luminosite utiliser text shadow
  */