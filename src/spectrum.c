#include "../headers/spectrum.h"
#include "../headers/listen.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>

#define FRAMES   1024  //one capture buffer (approx 23 ms at 44.1 kHz)
#define BAR_COLS 50

//set from the SIGINT handler so a blocking capture loop can stop cleanly
static volatile sig_atomic_t g_stop = 0;

static void on_sigint(int sig){
    (void)sig;
    g_stop = 1;
}

//maps an RMS amplitude in 0-1 range to a 0-BAR_COLS bar on a dBFS scale (-60 dB floor),
//so very quiet signal is still visible
static int meter_cols(float rms){
    if(rms <= 1e-6f){ //otherwise log 0 == -inf
        return 0;
    }
    float db = 20.0f * log10f(rms);     //<= 0 dBFS
    float frac = (db + 60.0f) / 60.0f;  //-60 dB -> 0, 0 dB -> 1
    if(frac < 0.0f) frac = 0.0f;
    if(frac > 1.0f) frac = 1.0f;
    return (int)(frac * BAR_COLS);
}

int run_spectrum(enum AppMode mode){
    audio_listener* listener = listen_open(NULL);
    if(!listener){
        return -1;  //reason already printed by listen_open
    }

    //Ctrl-C raises SIGINT -> sets g_stop var -> loop exits + cleanup
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    //raw and non-blocking stdin so we can poll for 'q' between audio buffers
    //without blocking the audio capture loop; ISIG stays on so Ctrl-C still fires
    struct termios original, raw;
    int have_tty = isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &original) == 0;
    if(have_tty){
        raw = original;
        raw.c_lflag &= ~(ICANON | ECHO);
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }

    if(mode == MODE_SPECTRUM_LYRICS){
        fputs("Level meter mode\n", stdout);
    }
    fputs("q or Ctrl-C to quit\n", stdout);

    float buf[FRAMES];
    while(!g_stop){
        if(listen_read(listener, buf, FRAMES) != 0){
            break;  //reason already printed by listen_read
        }

        if(have_tty){
            char key;
            if(read(STDIN_FILENO, &key, 1) == 1 && (key == 'q' || key == 'Q')){
                break;
            }
        }

        double sum_sq = 0.0;
        for(int i = 0; i < FRAMES; i++){
            sum_sq += (double)buf[i] * buf[i];
        }
        float rms = (float)sqrt(sum_sq / FRAMES);

        int cols = meter_cols(rms);
        printf("\r[");
        for(int c = 0; c < BAR_COLS; c++){
            putchar(c < cols ? '#' : ' ');
        }
        printf("] %6.4f", rms);
        fflush(stdout);
    }

    if(have_tty){
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
    }
    putchar('\n');
    listen_close(listener);
    return 0;
}
