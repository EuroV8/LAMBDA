#include "../headers/menu.h"
#include "../headers/display.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#define MODE_COUNT 2

//number of terminal lines the redrawable option block occupies:
//1 blank + MODE_COUNT options + 1 blank + 1 hint
#define OPTION_BLOCK_LINES (MODE_COUNT + 3)

//ANSI helpers
#define C_RESET     "\033[0m"
#define C_LOGO      "\033[1;36m"  //bold bright cyan
#define C_DIM       "\033[2m"
#define C_REVERSE   "\033[7m"     //highlight for the selected option
#define CURSOR_HIDE "\033[?25l"
#define CURSOR_SHOW "\033[?25h"

static const char* const LOGO[] = {
    "           ___  __   __ ____           ___  ",
    "    /\\    / _ \\|  \\ /  |  _ \\   /\\    / _ \\ ",
    "   /  \\  | |_| |   v   | |_) ) /  \\  | |_| |",
    "  / /\\ \\ |  _  | |\\_/| |  _ ( / /\\ \\ |  _  |",
    " / /  \\ \\| | | | |   | | |_) ) /__\\ \\| | | |",
    "/_/    \\_\\_| |_|_|   |_|____/________\\_| |_|",
};
#define LOGO_LINES ((int)(sizeof(LOGO) / sizeof(LOGO[0])))

static const char* const MODE_LABELS[MODE_COUNT] = {
    "Spectrum analyzer only",
    "Spectrum + lyrics",
};

//redraw the OPTION_BLOCK_LINES line showing an option IN-PLACE
//each line is cleared first so redraws leave no leftovers 
//the selected_idx option is shown highlighted
static void draw_options(int selected_idx){
    fputs("\r\033[K\n", stdout);
    for(int i = 0; i < MODE_COUNT; i++){
        if(i == selected_idx){
            printf("\r\033[K  " C_REVERSE " > %s " C_RESET "\n", MODE_LABELS[i]);
        }
        else{
            printf("\r\033[K     %s\n", MODE_LABELS[i]);
        }
    }
    fputs("\r\033[K\n", stdout);
    fputs("\r\033[K   " C_DIM "Up/Down move - Enter select - q quit" C_RESET "\n", stdout);
    fflush(stdout);
}

//prints the static splash header (logo + tagline + version/creator)
static void draw_header(void){
    clear();
    putchar('\n');
    fputs(C_LOGO, stdout); //set colour
    for(int i = 0; i < LOGO_LINES; i++){
        puts(LOGO[i]);
    }
    fputs(C_RESET, stdout); //reset print colour
    putchar('\n');
    fputs("   Lyric And Music Beautiful Display Aggregator\n", stdout);
    printf("   " C_DIM "%s   created by %s" C_RESET "\n", LAMBDA_VERSION, LAMBDA_CREATOR);
}

//read one keypress from raw-mode stdin
//returns: 'U' up, 'D' down, '\n' confirm, 'q' abort, 0 ignore
static int read_key(void){
    unsigned char key;
    if(read(STDIN_FILENO, &key, 1) != 1){
        return 'q';  //EOF
    }
    if(key == '\r' || key == '\n'){
        return '\n';
    }
    if(key == 'q' || key == 'Q' || key == 3){
        return 'q';  //q or Ctrl-C (ISIG is disabled so it arrives as a byte)
    }
    if(key == 'k' || key == 'K'){
        return 'U';
    }
    if(key == 'j' || key == 'J'){
        return 'D';
    }
    if(key == '\033'){
        unsigned char escape_seq[2];
        if(read(STDIN_FILENO, &escape_seq[0], 1) != 1){
            return 'q';
        }
        if(read(STDIN_FILENO, &escape_seq[1], 1) != 1){
            return 0;
        }
        if(escape_seq[0] == '['){
            if(escape_seq[1] == 'A'){
                return 'U';
            }
            if(escape_seq[1] == 'B'){
                return 'D';
            }
        }
        return 0;
    }
    return 0;
}

//non-interactive fallback: numbered prompt read line-by-line from stdin
static int menu_fallback(enum AppMode* out_mode){
    fputs("\n   Select a mode:\n", stdout);
    for(int i = 0; i < MODE_COUNT; i++){
        printf("     %d) %s\n", i + 1, MODE_LABELS[i]);
    }
    fputs("   > ", stdout);
    fflush(stdout);

    char line[64];
    if(!fgets(line, sizeof(line), stdin)){
        return -1;
    }
    int choice = line[0] - '0';
    if(choice < 1 || choice > MODE_COUNT){
        return -1;
    }
    *out_mode = (enum AppMode)(choice - 1);
    return 0;
}

int run_menu(enum AppMode* out_mode){
    if(!out_mode){
        return -1;
    }

    draw_header();

    if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)){
        return menu_fallback(out_mode);
    }

    struct termios original_term;
    if(tcgetattr(STDIN_FILENO, &original_term) == -1){
        return menu_fallback(out_mode);
    }
    struct termios raw_term = original_term;
    raw_term.c_lflag &= ~(ICANON | ECHO | ISIG);
    raw_term.c_cc[VMIN]  = 1;
    raw_term.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_term);
    fputs(CURSOR_HIDE, stdout);

    int selected_idx = 0;
    draw_options(selected_idx);

    int result = -1;
    for(;;){
        int key = read_key();
        if(key == 'q'){
            break;
        }
        if(key == '\n'){
            *out_mode = (enum AppMode)selected_idx;
            result = 0;
            break;
        }
        if(key == 'U'){
            selected_idx = (selected_idx + MODE_COUNT - 1) % MODE_COUNT;
        }
        else if(key == 'D'){
            selected_idx = (selected_idx + 1) % MODE_COUNT;
        }
        else{
            continue;  //ignored key, no repaint needed
        }
        printf("\033[%dA", OPTION_BLOCK_LINES);  //rewind over the option block
        draw_options(selected_idx);
    }

    fputs(CURSOR_SHOW, stdout);
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_term);
    putchar('\n');
    return result;
}
