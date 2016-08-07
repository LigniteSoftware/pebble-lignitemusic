#pragma once

#define IPOD_MESSAGE_DESTROY_TIME 500

#include <lignite_music.h>

/*
The next three functions are convenience functions to log using APP_LOG,
but skips the step of adding in the (almost) arbitrary first parameter.
 */
#define NSLog(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_INFO, __FILE_NAME__, __LINE__, fmt, ## args);

#define NSWarn(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_WARNING, __FILE_NAME__, __LINE__, fmt, ## args);

#define NSError(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_ERROR, __FILE_NAME__, __LINE__, fmt, ## args);

//The max amount of bytes that can be received from the phone at once.
//(always one less than the value on the companion app)
#define MAX_BYTES 1499

/**
 * The keys associated with data incoming and outgoing from and to the phone.
 */
typedef enum {
    MessageKeyReconnect = 0,
    MessageKeyRequestLibrary,
    MessageKeyRequestOffset,
    MessageKeyLibraryResponse,
    MessageKeyNowPlaying,
    MessageKeyRequestParent,
    MessageKeyPlayTrack,
    MessageKeyNowPlayingResponseType,
    MessageKeyAlbumArt,
    MessageKeyAlbumArtLength,
    MessageKeyAlbumArtIndex,
    MessageKeyChangeState,
    MessageKeyCurrentState,
    MessageKeySequenceNumber
} MessageKey;

/*
#define IPOD_RECONNECT_KEY 0xFEFF
#define IPOD_REQUEST_LIBRARY_KEY 0xFEFE
#define IPOD_REQUEST_OFFSET_KEY 0xFEFB
#define IPOD_LIBRARY_RESPONSE_KEY 0xFEFD
#define IPOD_NOW_PLAYING_KEY 0xFEFA
#define IPOD_REQUEST_PARENT_KEY 0xFEF9
#define IPOD_PLAY_TRACK_KEY 0xFEF8
#define IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY 0xFEF7
#define IPOD_ALBUM_ART_KEY 0xFEF6
#define IPOD_ALBUM_ART_LENGTH_KEY 0xFEF5
#define IPOD_ALBUM_ART_INDEX_KEY 0xFEF4
#define IPOD_CHANGE_STATE_KEY 0xFEF3
#define IPOD_CURRENT_STATE_KEY 0xFEF2
#define IPOD_SEQUENCE_NUMBER_KEY 0xFEF1
*/

/**
 * The type of media grouping.
 */
typedef enum {
    MPMediaGroupingTitle,
    MPMediaGroupingAlbum,
    MPMediaGroupingArtist,
    MPMediaGroupingAlbumArtist,
    MPMediaGroupingComposer,
    MPMediaGroupingGenre,
    MPMediaGroupingPlaylist,
    MPMediaGroupingPodcastTitle,
} MPMediaGrouping;

/**
 * The current playback state of the music.
 * This is the actual state from the phone, not the change of state seen in
 * the now playing window.
 */
typedef enum {
    MPMusicPlaybackStateStopped,
    MPMusicPlaybackStatePlaying,
    MPMusicPlaybackStatePaused,
    MPMusicPlaybackStateInterrupted,
    MPMusicPlaybackStateSeekingForward,
    MPMusicPlaybackStateSeekingBackward
} MPMusicPlaybackState;

/**
 * The repeat mode of the music.
 */
typedef enum {
    MPMusicRepeatModeDefault,
    MPMusicRepeatModeNone,
    MPMusicRepeatModeOne,
    MPMusicRepeatModeAll
} MPMusicRepeatMode;

/**
 * The shuffle mode of the music.
 */
typedef enum {
    MPMusicShuffleModeDefault,
    MPMusicShuffleModeOff,
    MPMusicShuffleModeSongs,
    MPMusicShuffleModeAlbums
} MPMusicShuffleMode;

/**
 * The different type of now playing data which is associated
 * with the currently playing music.
 */
typedef enum {
    NowPlayingTitle,
    NowPlayingArtist,
    NowPlayingAlbum,
    NowPlayingTitleArtist,
    NowPlayingNumbers,
} NowPlayingType;

/**
 * An iPodMessage is sent to the phone and is a convenience struct
 * to package in two crucial pieces of data for function returning.
 */
typedef struct {
    DictionaryIterator *iter;
    AppMessageResult result;
} iPodMessage;

/**
 * Animates a layer from one frame to another with a length and delay in ms.
 * @param layer  The layer to animate.
 * @param start  The starting frame pointer.
 * @param finish The final frame pointer.
 * @param length The length of the animation in milliseconds.
 * @param delay  The delay of the animation from calling this function to when it fires,
 * in milliseconds as well.
 */
void animate_layer(Layer *layer, GRect *start, GRect *finish, int length, int delay);

/**
 * Prepares for an iPodMessage to be sent to the phone.
 * @return The prepared iPodMessage pointer.
 */
iPodMessage *ipod_message_outbox_get();

/**
 * Destroys a previously created iPodMessage.
 * @param message The iPodMessage to destroy.
 */
void ipod_message_destroy(iPodMessage *message);

/**
 * Resets the sequence number of the message going through.
 */
void reset_sequence_number();