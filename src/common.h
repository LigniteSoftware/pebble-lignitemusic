#ifndef ipod_common_h
#define ipod_common_h

#define IPOD_MESSAGE_DESTROY_TIME 500

#include <pebble.h>

#define NSLog(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_INFO, __FILE_NAME__, __LINE__, fmt, ## args);

#define NSWarn(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_WARNING, __FILE_NAME__, __LINE__, fmt, ## args);

#define NSError(fmt, args...)                                \
  app_log(APP_LOG_LEVEL_ERROR, __FILE_NAME__, __LINE__, fmt, ## args);

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

typedef enum {
    MPMusicPlaybackStateStopped,
    MPMusicPlaybackStatePlaying,
    MPMusicPlaybackStatePaused,
    MPMusicPlaybackStateInterrupted,
    MPMusicPlaybackStateSeekingForward,
    MPMusicPlaybackStateSeekingBackward
} MPMusicPlaybackState;

typedef enum {
    MPMusicRepeatModeDefault, // the user's preference for repeat mode
    MPMusicRepeatModeNone,
    MPMusicRepeatModeOne,
    MPMusicRepeatModeAll
} MPMusicRepeatMode;

typedef enum {
    MPMusicShuffleModeDefault, // the user's preference for shuffle mode
    MPMusicShuffleModeOff,
    MPMusicShuffleModeSongs,
    MPMusicShuffleModeAlbums
} MPMusicShuffleMode;

typedef enum {
    NowPlayingTitle,
    NowPlayingArtist,
    NowPlayingAlbum,
    NowPlayingTitleArtist,
    NowPlayingNumbers,
} NowPlayingType;

typedef struct {
    DictionaryIterator *iter;
    AppMessageResult result;
} iPodMessage;

void animate_layer(Layer *layer, GRect *start, GRect *finish, int length, int delay);
iPodMessage *ipod_message_outbox_get();
void ipod_message_destroy(iPodMessage *message);
void reset_sequence_number();

#endif
