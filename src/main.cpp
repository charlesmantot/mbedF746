#include <mbed.h>
#include <threadLvgl.h>

#include "demos/lv_demos.h"
#include <cstdio>

ThreadLvgl threadLvgl(30);

int main() {

    threadLvgl.lock();//mutex a utilise pour les affichages

    lv_demo_widgets();//creation des affichages

    threadLvgl.unlock();//mutex a utilise pour les affichages

    while (1) {
        // put your main code here, to run repeatedly:
        ThisThread::sleep_for(10ms);
    }
}
