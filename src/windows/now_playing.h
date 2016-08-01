#pragma once

#include <lignite_music.h>

/**
 * NowPlayingState is what gets sent to the phone and tells it
 * the state to change or execute.
 */
typedef enum {
    NowPlayingStatePlayPause = 1,
    NowPlayingStateSkipNext,
    NowPlayingStateSkipPrevious,
    NowPlayingStateVolumeUp,
    NowPlayingStateVolumeDown
} NowPlayingState;

/**
 * Shows the now playing window. Soon to be deprecated.
 */
void now_playing_show();

/**
 * Ticks the now playing window, refreshing any data that may need
 * refreshing.
 */
void now_playing_tick();

/**
 * Ticks for the animation on the now playing window.
 */
void now_playing_animation_tick();

/**
 * Sets the album art of the now playing window.
 * @param album_art The album art to set. If NULL, a generic no-album-art cover
 * will be displayed automagically.
 */
void now_playing_set_album_art(GBitmap *album_art);