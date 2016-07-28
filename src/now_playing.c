#include <pebble.h>
#include "now_playing.h"
#include "marquee_text.h"
#include "ipod_state.h"
#include "ipod.h"
#include "progress_bar.h"
#include "common.h"
#include "gbitmap_manipulator.h"

Window *now_playing_window;
ActionBarLayer *action_bar;
MarqueeTextLayer title_layer;
MarqueeTextLayer album_layer;
MarqueeTextLayer artist_layer;
BitmapLayer *album_art_layer;
GBitmap *album_art_bitmap;
uint8_t *album_art_data;
ProgressBarLayer progress_bar;

// Action bar icons
GBitmap *icon_pause;
GBitmap *icon_play;
GBitmap *icon_fast_forward;
GBitmap *icon_rewind;
GBitmap *icon_volume_up;
GBitmap *icon_volume_down;

void click_config_provider(void *context);
void window_unload(Window* window);
void window_load(Window* window);
void clicked_up(ClickRecognizerRef recognizer, void *context);
void clicked_select(ClickRecognizerRef recognizer, void *context);
void long_clicked_select(ClickRecognizerRef recognizer, void *context);
void clicked_down(ClickRecognizerRef recognizer, void *context);
void request_now_playing();
void send_state_change(NowPlayingState change);

void app_in_received(DictionaryIterator *received, void *context);
void state_callback(bool track_data);

void display_no_album();

bool controlling_volume = false;
bool is_shown = false;
AppTimer *now_playing_timer;
int currentAlbumArtLength = 0;

void show_now_playing() {
    now_playing_window = window_create();
    window_set_window_handlers(now_playing_window, (WindowHandlers){
        .unload = window_unload,
        .load = window_load,
    });
    window_stack_push(now_playing_window, true);
}

void now_playing_tick() {
    progress_bar_layer_set_value(&progress_bar, ipod_state_current_time());
}

void now_playing_animation_tick() {
    if(!is_shown) {
        return;
    }

    //app_timer_cancel(now_playing_timer);
    now_playing_timer = app_timer_register(33, ipod_app_timer_handler, NULL);
}

void window_load(Window* window) {
    Layer *window_layer = window_get_root_layer(window);

    // Load bitmaps for action bar icons.
    icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
    icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
    icon_fast_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FAST_FORWARD);
    icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);
    icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
    icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);

    // Action bar
    action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(action_bar, window);
    action_bar_layer_set_click_config_provider(action_bar, click_config_provider);
    controlling_volume = false;
    // Set default icon set.
    action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_fast_forward);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_rewind);
    action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_play);

    // Text labels
    marquee_text_layer_init(&title_layer, GRect(2, 0, 112, 35));
    marquee_text_layer_set_font(&title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    marquee_text_layer_set_text(&title_layer, ipod_get_title());
    layer_add_child(window_layer, title_layer.layer);

    marquee_text_layer_init(&album_layer, GRect(2, 130, 112, 23));
    marquee_text_layer_set_font(&album_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(&album_layer, ipod_get_album());
    layer_add_child(window_layer, album_layer.layer);

    marquee_text_layer_init(&artist_layer, GRect(2, 107, 112, 28));
    marquee_text_layer_set_font(&artist_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    marquee_text_layer_set_text(&artist_layer, ipod_get_artist());
    layer_add_child(window_layer, artist_layer.layer);

    // Progress bar
    progress_bar_layer_init(&progress_bar, GRect(4, 105, 104, 7));
    layer_add_child(window_layer, progress_bar.layer);

    // Album art
    //TODO: make sure this is valid lmao
    //album_art_bitmap = gbitmap_create_with_data(&&album_art_data);
    /*
     * Never forget
     * Also be sure to check gbitmap_create_from_png_data just in case
     *
    album_art_bitmap = (GBitmap) {
        .addr = album_art_data,
        .bounds = GRect(0, 0, 64, 64),
        .info_flags = 1,
        .row_size_bytes = 8,
    };
    */

    //I didn't comment this out
    //memset(album_art_data, 0, 512);

    album_art_layer = bitmap_layer_create(GRect(20, 35, 64, 64));
    if(album_art_bitmap){
        bitmap_layer_set_bitmap(album_art_layer, album_art_bitmap);
    }
    layer_add_child(window_layer, bitmap_layer_get_layer(album_art_layer));
    display_no_album();

    //app_message_register_inbox_received(app_in_received);
    ipod_state_set_callback(state_callback);

    request_now_playing();

    is_shown = true;
    now_playing_animation_tick();
}

void window_unload(Window* window) {
    action_bar_layer_remove_from_window(action_bar);
    action_bar_layer_destroy(action_bar);
    marquee_text_layer_deinit(&title_layer);
    marquee_text_layer_deinit(&album_layer);
    marquee_text_layer_deinit(&artist_layer);

    // deinit action bar icons
    gbitmap_destroy(icon_pause);
    gbitmap_destroy(icon_play);
    gbitmap_destroy(icon_fast_forward);
    gbitmap_destroy(icon_rewind);
    gbitmap_destroy(icon_volume_up);
    gbitmap_destroy(icon_volume_down);

    ipod_state_set_callback(NULL);
    is_shown = false;
}

void click_config_provider(void* context) {
    window_single_click_subscribe(BUTTON_ID_DOWN, clicked_down);
    window_single_click_subscribe(BUTTON_ID_UP, clicked_up);
    window_single_click_subscribe(BUTTON_ID_SELECT, clicked_select);
    //TODO: make sure this is a good length for the long click
    window_long_click_subscribe(BUTTON_ID_SELECT, 500, long_clicked_select, NULL);
}

void clicked_up(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        send_state_change(NowPlayingStateSkipPrevious);
    }
    else {
        send_state_change(NowPlayingStateVolumeUp);
    }
}

void clicked_select(ClickRecognizerRef recognizer, void *context) {
    send_state_change(NowPlayingStatePlayPause);
}

void clicked_down(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        send_state_change(NowPlayingStateSkipNext);
    }
    else {
        send_state_change(NowPlayingStateVolumeDown);
    }
}

void long_clicked_select(ClickRecognizerRef recognizer, void *context) {
    controlling_volume = !controlling_volume;
    if(controlling_volume) {
        action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_volume_up);
        action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_volume_down);
    }
    else {
        action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_fast_forward);
        action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_rewind);
    }
}

void send_state_change(NowPlayingState state_change) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        return;
    }
    NSLog("Sending state change of %d", state_change);
    dict_write_int8(ipodMessage->iter, IPOD_CHANGE_STATE_KEY, state_change);
    NSLog("outbox state send result of %d", app_message_outbox_send());

    ipod_message_destroy(ipodMessage);
}

//TODO: dealloc ipodmessage struct
void request_now_playing() {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        NSError("Iter is null!!!");
        return;
    }
    NSLog("Requesting now playing");
    dict_write_int8(ipodMessage->iter, IPOD_NOW_PLAYING_KEY, 2);
    AppMessageResult result = app_message_outbox_send();
    NSLog("Outbox send result %d", result);
}

void create_bitmap(){
    album_art_bitmap = gbitmap_create_from_png_data(album_art_data, currentAlbumArtLength);
    if(album_art_layer){
        bitmap_layer_set_bitmap(album_art_layer, album_art_bitmap);
    }

    NSLog("Got bitmap format of %s", get_gbitmapformat_text(gbitmap_get_format(album_art_bitmap)));
}

void now_playing_process_album_art_tuple(DictionaryIterator *albumArtDict){
    Tuple *albumArtTuple = dict_find(albumArtDict, IPOD_ALBUM_ART_KEY);
    if(albumArtTuple) {
        if(!album_art_data){
            NSLog("Album art data doesn't exist!");
            //Send one more request for album art
            return;
        }

        Tuple *albumArtIndexTuple = dict_find(albumArtDict, IPOD_ALBUM_ART_INDEX_KEY);
        if(!albumArtIndexTuple){
            NSError("Index tuple doesn't exist!");
            return;
        }
        size_t index = albumArtIndexTuple->value->uint16 * 499;
        memcpy(&album_art_data[index], albumArtTuple->value->data, albumArtTuple->length);

        if(((albumArtIndexTuple->value->uint16 * 499) + albumArtTuple->length) == currentAlbumArtLength){
            NSLog("Firing timer");
            app_timer_register(125, create_bitmap, NULL);
        }
    }
    else{
        Tuple* albumArtLengthTuple = dict_find(albumArtDict, IPOD_ALBUM_ART_LENGTH_KEY);
        //TODO check this shit
        if(albumArtLengthTuple){
            uint16_t length = albumArtLengthTuple->value->uint16;

            NSLog("Got size for image %d", length);
            if(album_art_data){
                NSLog("Destroying album art data.");
                free(album_art_data);
            }
            if(album_art_bitmap){
                NSLog("Destroying album art bitmap.");
                bitmap_layer_set_bitmap(album_art_layer, NULL);
                gbitmap_destroy(album_art_bitmap);
                album_art_bitmap = NULL;
            }
            album_art_data = malloc(length);
            if(!album_art_data){
                NSError("Album art data FAILED to create with a size of %d bytes!", length);
            }
            currentAlbumArtLength = length;

            //You're probably not gonna have a PNG that's 1 byte lol, so if it is it's the phone telling you
            //that there's no fucking album art
            if(currentAlbumArtLength == 1){
                display_no_album();
            }
            else{
                NSLog("Ready for image input.");
            }
        }
    }
}

void state_callback(bool track_data) {
    if(!is_shown){
        return;
    }
    NSLog("Got %d", ipod_state_duration());
    if(track_data) {
        NSLog("Updating marquee layers");
        marquee_text_layer_set_text(&album_layer, ipod_get_album());
        marquee_text_layer_set_text(&artist_layer, ipod_get_artist());
        marquee_text_layer_set_text(&title_layer, ipod_get_title());
    }
    else {
        NSLog("Updating icons");
        if(ipod_get_playback_state() == MPMusicPlaybackStatePlaying) {
            action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_pause);
        } else {
            action_bar_layer_set_icon(action_bar, BUTTON_ID_SELECT, icon_play);
        }
        progress_bar_layer_set_range(&progress_bar, 0, ipod_state_duration());
        progress_bar_layer_set_value(&progress_bar, ipod_state_current_time());
    }
}

void display_no_album() {
    //TODO: this is where the shit goes that has album art missing
    //resource_load(resource_get_handle(RESOURCE_ID_ALBUM_ART_MISSING), album_art_data, 512);
    NSLog("Display no album");
    layer_set_frame(bitmap_layer_get_layer(album_art_layer), GRect(20, 35, 64, 64));
    layer_mark_dirty(bitmap_layer_get_layer(album_art_layer));
}
