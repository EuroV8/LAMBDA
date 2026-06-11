#ifndef LISTEN_H
#define LISTEN_H

#include <stddef.h>

//capture defaults: the spectrum analyzes a steady mono float stream of
//whatever is currently playing out the speakers (the system audio mix)
#define LISTEN_SAMPLE_RATE 44100
#define LISTEN_CHANNELS    1

//opaque capture handle, one per open monitor stream
typedef struct audio_listener audio_listener;

//Open a blocking capture stream on a system audio monitor source
//device : backend source name, or NULL to capture the default sink's monitor
//  (ie. the mix the user actually hears). On Linux this is the PulseAudio
//  "@DEFAULT_MONITOR@" source.
//Returns a handle (CALLER closed) or NULL on failure (reason dumped to stderr)
//Linux-only
audio_listener* listen_open(const char* device);

//Read "frames" amount of samples in range -1,1 to "out", having a handle (blocking)
//until they're captured
//Returns 0 on success, -1 on error
int listen_read(audio_listener* listener, float* out, size_t frames);

//Closes the stream and frees the listener handle
void listen_close(audio_listener* listener);

#endif
