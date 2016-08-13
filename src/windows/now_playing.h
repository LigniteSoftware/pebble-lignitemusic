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
 * Whether or not the now playing window is already on the stack (created).
 * @return boolean of whether or not the above is true.
 */
bool now_playing_is_shown();

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
 * @param image_part The part of the image to be set.
 * @param album_art  The album art to set. If NULL, a generic no-album-art cover
 * will be displayed automagically.
 */
void now_playing_set_album_art(uint8_t image_part, GBitmap *album_art);