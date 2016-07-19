#include <pebble.h>
#include "ipod_state.h"
#include "common.h"

void ipod_received_handler(DictionaryIterator *received, void *context);
void call_callback(bool track_data);

iPodStateCallback state_callback;

MPMusicRepeatMode s_repeat_mode;
MPMusicShuffleMode s_shuffle_mode;
MPMusicPlaybackState s_playback_state;
uint32_t s_duration;
uint32_t s_current_time;
static char s_album[100];
static char s_artist[100];
static char s_title[100];

void ipod_state_init() {
    app_message_register_inbox_received(ipod_received_handler);
    s_artist[0] = '\0';
    s_album[0] = '\0';
    s_title[0] = '\0';
}

void ipod_state_tick() {
    if(s_playback_state == MPMusicPlaybackStatePlaying) {
        if(s_current_time < s_duration) {
            ++s_current_time;
        } else {
            iPodMessage *ipodMessage = ipod_message_outbox_get();
            if(!ipodMessage->iter){
                free(ipodMessage);
                return;
            }
            dict_write_int8(ipodMessage->iter, IPOD_NOW_PLAYING_KEY, 2);
            app_message_outbox_send();
        }
    }
}

void ipod_state_set_callback(iPodStateCallback callback) {
    state_callback = callback;
}

uint16_t ipod_state_current_time() {
    return s_current_time;
}

uint16_t ipod_state_duration() {
    return s_duration;
}

MPMusicPlaybackState ipod_get_playback_state() {
    return s_playback_state;
}

MPMusicRepeatMode ipod_get_repeat_mode() {
    return s_repeat_mode;
}

MPMusicShuffleMode ipod_get_shuffle_mode() {
    return s_shuffle_mode;
}

char* ipod_get_album() {
    return s_album;
}

char* ipod_get_artist() {
    return s_artist;
}

char* ipod_get_title() {
    return s_title;
}

void ipod_set_shuffle_mode(MPMusicShuffleMode shuffle) {}
void ipod_set_repeat_mode(MPMusicRepeatMode repeat) {}

void call_callback(bool track_data) {
    if(!state_callback) return;
    state_callback(track_data);
}

void ipod_received_handler(DictionaryIterator *received, void *context) {
    Tuple *tuple = dict_find(received, IPOD_CURRENT_STATE_KEY);
    if(tuple) {
        s_playback_state = tuple->value->data[0];
        s_shuffle_mode = tuple->value->data[1];
        s_repeat_mode = tuple->value->data[2];
        s_duration = (tuple->value->data[3] << 8) | tuple->value->data[4];
        s_current_time = (tuple->value->data[5] << 8) | tuple->value->data[6];
        call_callback(false);
        return;
    }
    tuple = dict_find(received, IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY);
    if(tuple) {
        NowPlayingType type = tuple->value->uint8;
        tuple = dict_find(received, IPOD_NOW_PLAYING_KEY);
        if(!tuple) return;
        char* target = NULL;
        switch(type) {
            case NowPlayingAlbum:
                target = s_album;
                break;
            case NowPlayingArtist:
                target = s_artist;
                break;
            case NowPlayingTitle:
                target = s_title;
                break;
            default:
                return;
        }
        if(strcmp(target, tuple->value->cstring) == 0) return;
        uint8_t len = strlen(tuple->value->cstring);
        if(len > 99) len = 99;
        memcpy(target, tuple->value->cstring, len);
        target[len] = '\0';
        call_callback(true);
        return;
    }
}
