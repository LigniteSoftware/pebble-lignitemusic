#include <pebble.h>
#include "ipod_state.h"
#include "common.h"
#include "now_playing.h"

void ipod_received_handler(DictionaryIterator *received, void *context);
void call_callback(bool track_data);

iPodStateCallback ipod_state_callback;

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
    ipod_state_callback = callback;
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
    if(!ipod_state_callback){
        NSError("State callback doesn't exist! Gutlessly rejecting.");
        return;
    }
    ipod_state_callback(track_data);
}

NowPlayingType currentType;

void process_tuple(Tuple *tuple, DictionaryIterator *iter){
    uint32_t key = tuple->key;
    if(key == IPOD_CURRENT_STATE_KEY){
        NSLog("Is playback info.");
        s_playback_state = tuple->value->data[0];
        s_shuffle_mode = tuple->value->data[1];
        s_repeat_mode = tuple->value->data[2];
        s_duration = (tuple->value->data[3] << 8) | tuple->value->data[4];
        s_current_time = (tuple->value->data[5] << 8) | tuple->value->data[6];
        NSLog("Updating callback.");
        call_callback(false);
    }
    else if(key == IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY){
        currentType = tuple->value->uint8;
    }
    else if(key == IPOD_NOW_PLAYING_KEY){
        Tuple *typeTuple = dict_find(iter, IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY);
        if(!typeTuple){
            NSError("Type tuple doesn't exist!");
            return;
        }
        currentType = typeTuple->value->uint8;

        char* target = NULL;
        switch(currentType) {
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
        if(strcmp(target, tuple->value->cstring) == 0){
            //NSWarn("Warning: Target string and cstring are equal. Not copying again.");
            return;
        }
        uint8_t len = strlen(tuple->value->cstring);
        if(len > 99){
            len = 99;
        }
        strncpy(target, tuple->value->cstring, (size_t)len);
        target[len] = '\0';
        call_callback(true);
    }
    else{
        now_playing_process_album_art_tuple(iter);
    }
}

void ipod_received_handler(DictionaryIterator *iter, void *context){
    NSLog("Got message from phone");
    Tuple *t = dict_read_first(iter);
    if(t){
        process_tuple(t, iter);
    }
    while(t != NULL){
        t = dict_read_next(iter);
        if(t){
            process_tuple(t, iter);
        }
    }
}

/*
void ipod_received_handler(DictionaryIterator *received, void *context) {
    NSLog("Got ipod message");
    Tuple *tuple = dict_find(received, IPOD_CURRENT_STATE_KEY);
    if(tuple) {
        NSLog("Is playback information.");
        s_playback_state = tuple->value->data[0];
        s_shuffle_mode = tuple->value->data[1];
        s_repeat_mode = tuple->value->data[2];
        s_duration = (tuple->value->data[3] << 8) | tuple->value->data[4];
        s_current_time = (tuple->value->data[5] << 8) | tuple->value->data[6];
        NSLog("Callback...");
        call_callback(false);
        return;
    }
    tuple = dict_find(received, IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY);
    if(tuple) {
        NSLog("Got now playing response");
        NowPlayingType type = tuple->value->uint8;
        tuple = dict_find(received, IPOD_NOW_PLAYING_KEY);
        if(!tuple){
            NSError("Tuple doesn't exist!");
            return;
        }
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
        if(strcmp(target, tuple->value->cstring) == 0){
            //NSWarn("Warning: Target string and cstring are equal. Not copying again.");
            return;
        }
        uint8_t len = strlen(tuple->value->cstring);
        if(len > 99){
            len = 99;
        }
        strncpy(target, tuple->value->cstring, (size_t)len);
        target[len] = '\0';
        call_callback(true);
        return;
    }
    else{
        now_playing_process_album_art_tuple(received);
    }
}
*/