#include "../headers/menu.h"
#include "../headers/display.h"
#include "../headers/spectrum.h"
#include <stdio.h>

int main(void){
    enum AppMode mode;
    if(run_menu(&mode) != 0){
        clear();
        fputs("LAMBDA: cancelled.\n", stdout);
        return 0;
    }

    clear();
    switch(mode){
        case MODE_SPECTRUM_ONLY:{
            fputs("Starting LAMBDA in spectrum analyzer mode...\n", stdout);
            break;
        }
        case MODE_SPECTRUM_LYRICS:{
            fputs("Starting LAMBDA in spectrum + lyrics mode...\n", stdout);
            break;
        }
    }

    if(run_spectrum(mode) != 0){
        return 1;
    }
    return 0;
}
