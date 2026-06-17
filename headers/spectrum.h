#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "menu.h"

//runs the visualizer in a given mode
//wrapper for opening the audio listener and displaying the visuals in the terminal
//returns 0 on "normal" (q,CTRL+C) exit or -1 if the listener couldn't be opened
int run_spectrum(enum AppMode mode);

#endif
