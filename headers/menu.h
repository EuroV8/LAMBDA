#ifndef MENU_H
#define MENU_H

#define LAMBDA_VERSION "v0.1.0"
#define LAMBDA_CREATOR "EuroV8"

enum AppMode {
    MODE_SPECTRUM_ONLY,
    MODE_SPECTRUM_LYRICS
};

//Displays the main menu, choose with arrow keys and enter
//writes chosen mode to out_mode on success
//returns 0 on success, -1 on failure
int run_menu(enum AppMode* out_mode);

#endif
