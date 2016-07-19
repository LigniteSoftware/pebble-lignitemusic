#include <pebble.h>
#include "now_playing.h"
#include "marquee_text.h"
#include "ipod_state.h"
#include "ipod.h"
#include "progress_bar.h"
#include "common.h"

Window *window;
ActionBarLayer *action_bar;
MarqueeTextLayer title_layer;
MarqueeTextLayer album_layer;
MarqueeTextLayer artist_layer;
BitmapLayer *album_art_layer;
GBitmap *album_art_bitmap;
uint8_t album_art_data[512];
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
void send_state_change(int8_t change);

void app_in_received(DictionaryIterator *received, void *context);
void state_callback(bool track_data);

void display_no_album();

bool controlling_volume = false;
bool is_shown = false;
AppTimer *now_playing_timer;

void show_now_playing() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers){
        .unload = window_unload,
        .load = window_load,
    });
    window_stack_push(window, true);
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
    marquee_text_layer_set_text(&title_layer, "ipod_get_title()");
    layer_add_child(window_layer, title_layer.layer);

    marquee_text_layer_init(&album_layer, GRect(2, 130, 112, 23));
    marquee_text_layer_set_font(&album_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(&album_layer, "ipod_get_album()");
    layer_add_child(window_layer, album_layer.layer);

    marquee_text_layer_init(&artist_layer, GRect(2, 107, 112, 28));
    marquee_text_layer_set_font(&artist_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    marquee_text_layer_set_text(&artist_layer, "ipod_get_artist()");
    layer_add_child(window_layer, artist_layer.layer);

    // Progress bar
    progress_bar_layer_init(&progress_bar, GRect(10, 105, 104, 7));
    layer_add_child(window_layer, progress_bar.layer);

    // Album art
    //TODO: make sure this is valid lmao
    //album_art_bitmap = gbitmap_create_with_data(&&album_art_data);
    album_art_bitmap = gbitmap_create_blank(GSize(40, 40), GBitmapFormat8Bit);
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
    bitmap_layer_set_bitmap(album_art_layer, album_art_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(album_art_layer));
    display_no_album();

    app_message_register_inbox_received(app_in_received);
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
        send_state_change(-1);
    } else {
        send_state_change(64);
    }
}
void clicked_select(ClickRecognizerRef recognizer, void *context) {
    send_state_change(0);
}
void clicked_down(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        send_state_change(1);
    } else {
        send_state_change(-64);
    }
}
void long_clicked_select(ClickRecognizerRef recognizer, void *context) {
    controlling_volume = !controlling_volume;
    if(controlling_volume) {
        action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_volume_up);
        action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_volume_down);
    } else {
        action_bar_layer_set_icon(action_bar, BUTTON_ID_DOWN, icon_fast_forward);
        action_bar_layer_set_icon(action_bar, BUTTON_ID_UP, icon_rewind);
    }
}

void send_state_change(int8_t state_change) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        return;
    }
    dict_write_int8(ipodMessage->iter, IPOD_STATE_CHANGE_KEY, state_change);
    app_message_outbox_send();
}

void request_now_playing() {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        NSError("Iter is null!!!");
        return;
    }
    dict_write_int8(ipodMessage->iter, IPOD_NOW_PLAYING_KEY, 2);
    app_message_outbox_send();
}

void app_in_received(DictionaryIterator *received, void* context) {
    if(!is_shown){
        return;
    }

    /*
    Tuple* tuple = dict_find(received, IPOD_ALBUM_ART_KEY);
    //todo fix this shit

    if(tuple) {
        if(tuple->value->data[0] == 255) {
            display_no_album();
        } else {
            size_t offset = tuple->value->data[0] * 104;
            memcpy(album_art_data + offset, tuple->value->data + 1, tuple->length - 1);
            layer_mark_dirty(&album_art_layer.layer);
            layer_set_frame((Layer*)&album_art_layer, GRect(30, 35, 64, 64));
        }
    }
    */
    APP_LOG(APP_LOG_LEVEL_INFO, "I got some data but I am gonna do nothing");
}

void state_callback(bool track_data) {
    if(!is_shown){
        return;
    }
    if(track_data) {
        marquee_text_layer_set_text(&album_layer, ipod_get_album());
        marquee_text_layer_set_text(&artist_layer, ipod_get_artist());
        marquee_text_layer_set_text(&title_layer, ipod_get_title());
    } else {
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
    //Todo: don't cast these
    layer_set_frame((Layer*)album_art_layer, GRect(20, 35, 64, 64));
    layer_mark_dirty((Layer*)album_art_layer);
}
