#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

void lv_example_label_2(void);
AnalogIn sensor(A0); //pin 9 de l'arduino pour capteur
DigitalOut buzzer(D2); //pin 17 de l'arduino pour buzzer

int main() {

    buzzer=1; //buzzer à 1 (bruit)
    buzzer=0; //buzzer à 0 (pas de bruit)
    threadLvgl.lock(); //mutex a utilise pour les affichages
    lv_example_label_2(); //affichage
    //lv_demo_widgets(); //affichages démo
    threadLvgl.unlock(); //mutex a utilise pour les affichages
    while (1) {
        // put your main code here, to run repeatedly:
        //ThisThread::sleep_for(10ms);//tempo 10ms
        

        threadLvgl.lock(); //mutex a utilise pour les affichages
        threadLvgl.unlock(); //mutex a utilise pour les affichages
    }
}
void lv_example_label_2(void)//fonction pour afficher sur un écran https://docs.lvgl.io/8.3/widgets/core/label.html?highlight=label
{
    /*Create a style for the shadow*/
    static lv_style_t style_shadow;
    lv_style_init(&style_shadow);
    lv_style_set_text_opa(&style_shadow, LV_OPA_30);
    lv_style_set_text_color(&style_shadow, lv_color_black());

    /*Create a label for the shadow first (it's in the background)*/
    lv_obj_t * shadow_label = lv_label_create(lv_scr_act());
    lv_obj_add_style(shadow_label, &style_shadow, 0);

    /*Create the main label*/
    lv_obj_t * main_label = lv_label_create(lv_scr_act());
    lv_label_set_text(main_label, "A simple method to create\n"
                      "shadows on a text.\n"
                      "It even works with\n\n"
                      "newlines     and spaces.");

    /*Set the same text for the shadow label*/
    lv_label_set_text(shadow_label, lv_label_get_text(main_label));

    /*Position the main label*/
    lv_obj_align(main_label, LV_ALIGN_CENTER, 0, 0);

    /*Shift the second label down and to the right by 2 pixel*/
    lv_obj_align_to(shadow_label, main_label, LV_ALIGN_TOP_LEFT, 2, 2);
}

//idee :
//  Pour le clavier numerique utiliser text auto-formatting
//  Pour affichage heure actu utiliser a clock from a meter
//  Pour affichage luminosite utiliser text shadow
