#include "../headers/menu.h"
#include "../headers/display.h"
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
    //TODO: call (and implement first..) the corresponding mode's wrappersś
    return 0;
}
