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
void lv_affiche_text(float valSensor); //Fonction pour l'affichage de la luminosité
void lv_affiche_time(void); // Fonction pour affichage de l'heure actuelle
void clavier(void); // Fonction de création du clavier
void lv_affiche_saisie(void); // Fonction pour afficher la saisie
static void orga_saisie(lv_event_t * e); // Fonction pour l'organisation de la saisie
static void ta_event_cb(lv_event_t * e); // Fonction pour le rappel des évennements
static lv_obj_t * kb;
static lv_obj_t * time_label;
static lv_obj_t * ta;
static lv_obj_t * saisie_label;
static char saisie_clavier[6]; // Variable pour stocker la saisie du clavier

// Déclaration E/S
AnalogIn sensor(A0); // Capteur de luminosité sur A0
DigitalOut buzzer(D2); // Buzzer sur D2
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
    lv_affiche_saisie(); // Initialiser l'affichage de la saisie
    threadLvgl.unlock();
    
    while (1) {
        valSensor = sensor.read(); // Lire la valeur du capteur (0.0 à 1.0)
        
        threadLvgl.lock(); // Prise du mutex pour les affichages LVGL
        lv_affiche_text(valSensor); // Affichage de la valeur
        lv_affiche_time(); // Affichage de l'heure
        lv_affiche_saisie(); // Affichage de la saisie
        threadLvgl.unlock(); // Relacher le mutex pour les affichages LVGL

        if(valSensor < 0.5){ // Si valeur du capteur inférieur à 0.5 alors le buzzer sonne
            buzzer = 1;
        }
        else { // Sinon il ne sonne pas
            buzzer = 0;
        }
        ThisThread::sleep_for(100ms); // Tempo de 100 ms
    }
}

// Fonction pour afficher la valeur du capteur sur un écran LVGL
void lv_affiche_text(float valSensor) {
    char buf[32];
    sprintf(buf, "Luminosite : %2.2f", valSensor); // Conversion de la valeur du capteur en chaîne de caractères

    // Création d'un style pour l'ombre
    static lv_style_t style_shadow;
    lv_style_init(&style_shadow);
    lv_style_set_text_opa(&style_shadow, LV_OPA_30);
    lv_style_set_text_color(&style_shadow, lv_color_black());

    // Création d'un label pour l'ombre (en arrière-plan)
    static lv_obj_t *shadow_label = nullptr;
    if (shadow_label == nullptr) {
        shadow_label = lv_label_create(lv_scr_act());
        lv_obj_add_style(shadow_label, &style_shadow, 0);
    }

    // Création du label principal
    static lv_obj_t *main_label = nullptr;
    if (main_label == nullptr) {
        main_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(main_label, buf); // Affichage de la valeur du capteur

    // Mettre à jour le texte pour le label de l'ombre
    lv_label_set_text(shadow_label, lv_label_get_text(main_label));

    // Positionner le label principal
    lv_obj_align(main_label, LV_ALIGN_TOP_RIGHT, -10, 10); // Affiche en haut à droite de l'écran

    // Décaler le label de l'ombre vers le bas et à droite de 2 pixels
    lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);
}

// Fonction pour afficher l'heure actuelle sur un écran LVGL
void lv_affiche_time(void) {
    time_t now = time(NULL); // Récupérer l'heure actuelle
    struct tm *local_time = localtime(&now);

    char buf[32];
    strftime(buf, sizeof(buf), "Heure actuelle : %H:%M:%S", local_time); // Conversion de l'heure en chaîne de caractères

    // Création ou mise à jour du label pour l'heure
    if (time_label == nullptr) {
        time_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(time_label, buf);

    // Positionner l'heure en haut à gauche
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 10, 10);
}

// Fonction pour afficher la saisie du clavier sur un écran LVGL
void lv_affiche_saisie(void) { 
    char buf[32];
    snprintf(buf, sizeof(buf), "Heure saisie : %s", saisie_clavier); // Conversion de la saisie en chaîne de caractères

    // Création ou mise à jour du label pour la saisie
    if (saisie_label == nullptr) {
        saisie_label = lv_label_create(lv_scr_act());
    }
    lv_label_set_text(saisie_label, buf);

    // Positionner la saisie en dessous de l'heure actuelle
    lv_obj_align(saisie_label, LV_ALIGN_TOP_LEFT, 10, 40);
}

// Fonction pour initialiser le clavier numérique
void clavier(void) {
    // Création de la zone de texte
    ta = lv_textarea_create(lv_scr_act());
    lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_textarea_set_accepted_chars(ta, "0123456789:"); // Caractères acceptés
    lv_textarea_set_max_length(ta, 5); // Longueur maximale de 5 caractères
    lv_textarea_set_one_line(ta, true); // Mode une ligne
    lv_textarea_set_text(ta, "");

    // Création d'un clavier 
    kb = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(kb, LV_HOR_RES, LV_VER_RES / 2); // Taille du clavier
    lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER); // Mode numérique
    lv_keyboard_set_textarea(kb, ta); // Associer le clavier à la zone de texte

    lv_obj_align(ta, LV_ALIGN_TOP_MID, 0, 50); // Aligner au milieu en haut avec un offset
}

// Fonction pour organiser la saisie dans la zone de texte
static void orga_saisie(lv_event_t * e) {
    lv_obj_t * ta = lv_event_get_target(e);
    const char * txt = lv_textarea_get_text(ta);
    if(txt[0] >= '0' && txt[0] <= '9' &&
       txt[1] >= '0' && txt[1] <= '9' &&
       txt[2] != ':') {
        lv_textarea_set_cursor_pos(ta, 2);
        lv_textarea_add_char(ta, ':'); // Ajouter ':' après deux chiffres
    }
}

// Fonction de rappel pour les événements de la zone de texte
static void ta_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * ta = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        orga_saisie(e);
    } else if(code == LV_EVENT_READY) {
        // Entrée a été appuyée.
        const char * txt = lv_textarea_get_text(ta);
        snprintf(saisie_clavier, sizeof(saisie_clavier), "%s", txt);
        printf("Text: %s\n", saisie_clavier); // Affiche la saisie dans le terminal
        lv_textarea_set_text(ta, ""); // Réinitialiser la zone de texte
    }
}

/*idée :
  Pour le clavier numérique utiliser text auto-formatting
  Pour affichage heure actuelle utiliser a clock from a meter
  Pour affichage luminosité utiliser text shadow
  */
