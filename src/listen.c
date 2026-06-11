#include "../headers/listen.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__

#include <pulse/simple.h>
#include <pulse/error.h>

//resolves default audio source (records what the user hears not one specific output stream)
//pipewire should work too through pulseaudio's compat layer
#define DEFAULT_MONITOR "@DEFAULT_MONITOR@"

struct audio_listener {
    pa_simple* pa;
    size_t     frame_bytes; //bytes per frame == channels * sizeof(float)
};

audio_listener* listen_open(const char* device){

    //Native Endian float32 keeps samples already in -1, 1 range
    //PulseAudio remixes the (usually stereo) monitor down to LISTEN_CHANNELS
    const pa_sample_spec ss = {
        .format   = PA_SAMPLE_FLOAT32NE,
        .rate     = LISTEN_SAMPLE_RATE,
        .channels = LISTEN_CHANNELS,
    };

    int error;
    pa_simple* pa = pa_simple_new(
        NULL,                                //default audio server
        "LAMBDA",                            //application name (shown in mixer)
        PA_STREAM_RECORD,
        device ? device : DEFAULT_MONITOR,   //source: caller's, else system mix
        "system audio",                      //stream desc
        &ss,
        NULL,                                //default channel map
        NULL,                                //default buffering (low-latency enough)
        &error);
    if(!pa){
        fprintf(stderr, "listen: cannot open capture stream: %s\n", pa_strerror(error));
        return NULL;
    }

    audio_listener* listener = malloc(sizeof(*listener));
    if(!listener){
        pa_simple_free(pa);
        fputs("listen: out of memory\n", stderr);
        return NULL;
    }
    listener->pa = pa;
    listener->frame_bytes = (size_t)ss.channels * sizeof(float);
    return listener;
}

int listen_read(audio_listener* listener, float* out, size_t frames){
    if(!listener || !out){
        return -1;
    }
    int error;
    //pa_simple_read blocks until the whole request is satisfied
    if(pa_simple_read(listener->pa, out, frames * listener->frame_bytes, &error) < 0){
        fprintf(stderr, "listen: read failed: %s\n", pa_strerror(error));
        return -1;
    }
    return 0;
}

void listen_close(audio_listener* listener){
    if(!listener){
        return;
    }
    if(listener->pa){
        pa_simple_free(listener->pa);
    }
    free(listener);
}

#else //!__linux__

struct audio_listener{ 
    int unused; 
};

audio_listener* listen_open(const char* device){
    (void)device;
    fputs("listen: system audio capture is only supported on Linux\n", stderr);
    return NULL;
}

int listen_read(audio_listener* listener, float* out, size_t frames){
    (void)listener; (void)out; (void)frames;
    return -1;
}

void listen_close(audio_listener* listener){
    (void)listener;
}

#endif
