#include <pebble.h>

#ifndef ipod_now_playing_h
#define ipod_now_playing_h

typedef enum {
    NowPlayingStatePlayPause = 1,
    NowPlayingStateSkipNext,
    NowPlayingStateSkipPrevious,
    NowPlayingStateVolumeUp,
    NowPlayingStateVolumeDown
} NowPlayingState;

void show_now_playing();
void now_playing_tick();
void now_playing_animation_tick();

#endif