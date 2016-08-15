#pragma once

#include <lignite_music.h>

/**
 * The iPodStateCallback is registered to allow something to know when the iPod
 * state changes, so it can refresh itself with the new data.
 * @param  track_data Whether or not it was track data that was updated.
 */
typedef void(*iPodStateCallback)(bool track_data);

/**
 * Destroys all album art that's currently loaded (and sets the image layers
 * data to NULL) to save memory. 
 */
void destroy_all_album_art();

/**
 * Creates the iPod state for use.
 */
void ipod_state_create();

/**
 * Ticks the iPod state, which refreshes any data associated.
 */
void ipod_state_tick();

/**
 * Sets the iPodStateCallback associated so when new data comes in, it is refreshed.
 * @param iPodStateCallback The iPodStateCallback to include.
 */
void ipod_state_set_callback(iPodStateCallback);

/**
 * Gets the current time of the track playing in seconds.
 * @return The time of the track playing in seconds.
 */
uint16_t ipod_state_current_time();

/**
 * Gets the total duration of the track playing in seconds.
 * @return The total duration of the track playing in seconds.
 */
uint16_t ipod_state_duration();

/**
 * Gets the current playback state of the track.
 * @return The current playback state of the track.
 */
MPMusicPlaybackState ipod_get_playback_state();

/**
 * Gets the current repeat mode of the track.
 * @return The current repeat mode of the track.
 */
MPMusicRepeatMode ipod_get_repeat_mode();

/**
 * Gets the current shuffle mode of the track.
 * @return The current shuffle mode of the track.
 */
MPMusicShuffleMode ipod_get_shuffle_mode();

/**
 * Gets the current album name.
 * @return The text of the current album name.
 */
char* ipod_get_album();

/**
 * Gets the current artist name.
 * @return The text of the current artist name.
 */
char* ipod_get_artist();

/**
 * Gets the current track title playing.
 * @return The text of the track title playing.
 */
char* ipod_get_title();

/**
 * Sets the shuffle mode of the music player.
 * @param shuffle The shuffle mode to set to.
 */
void ipod_set_shuffle_mode(MPMusicShuffleMode shuffle);

/**
 * Sets the repeat mode of the music player.
 * @param repeat The repeat mode to set to.
 */
void ipod_set_repeat_mode(MPMusicRepeatMode repeat);

/**
 * Sends a request to the phone asking for the current now playing data.
 * @param force_reload Whether or not the phone should ignore logic (ie. sending
 * album art image twice) and instead just make sure it sends all data required.
 */
void now_playing_request(bool force_reload);
