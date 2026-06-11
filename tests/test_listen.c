#include "../headers/listen.h"
#include <stdio.h>
#include <math.h>

//Opens the default sys audio monitor, reads some buffers and prints an RMS level
//meter (live). To test properly you need to play some audio
#define FRAMES   1024
#define READS    200            //~200 * 1024 / 44100 Hz ~= 4.6 s of capture
#define BAR_COLS 50

//map an RMS amplitude in 0, 1 to a 0, BAR_COLS bar length on a dBFS scale
//(-60 dB floor), so quiet audible signal is still visible
static int meter_cols(float rms){
    if(rms <= 1e-6f){
        return 0;
    }
    float db = 20.0f * log10f(rms);     //<= 0 dBFS
    float frac = (db + 60.0f) / 60.0f;  //-60 dB -> 0, 0 dB -> 1
    if(frac < 0.0f) frac = 0.0f;
    if(frac > 1.0f) frac = 1.0f;
    return (int)(frac * BAR_COLS);
}

int main(void){
    audio_listener* al = listen_open(NULL);
    if(!al){
        return 1;
    }

    float buf[FRAMES];
    for(int r = 0; r < READS; r++){
        if(listen_read(al, buf, FRAMES) != 0){
            listen_close(al);
            return 1;
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

    putchar('\n');
    listen_close(al);
    return 0;
}
