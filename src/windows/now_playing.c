#include <lignite_music.h>

void now_playing_action_bar_handle(bool is_select);

Window *now_playing_window;

ActionBarLayer *control_action_bar;
BitmapLayer *now_playing_album_art_layer;
Layer *now_playing_graphics_layer;
MarqueeTextLayer *now_playing_title_layer, *now_playing_album_layer, *now_playing_artist_layer;
ProgressBarLayer *now_playing_progress_bar;

GBitmap *now_playing_album_art_bitmap, *no_album_art_bitmap;

AppTimer *now_playing_timer, *action_bar_timer;
GColor background_colour;
GFont artist_font;

Settings now_playing_settings;

static bool controlling_volume = false, is_shown = false;
static GBitmap *icon_pause, *icon_play, *icon_fast_forward, *icon_rewind, *icon_volume_up, *icon_volume_down;

/*
 * When there is no album art available (either still loading or the phone has said so), call this
 * to display a basic coverart
 */
void display_no_album() {
    bitmap_layer_set_bitmap(now_playing_album_art_layer, no_album_art_bitmap);
}

bool now_playing_is_playing_music(){
    return ipod_get_playback_state() == MPMusicPlaybackStatePlaying;
}

void now_playing_send_state_change(NowPlayingState state_change) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        return;
    }

    dict_write_int8(ipodMessage->iter, MessageKeyChangeState, state_change);
    app_message_outbox_send();

    ipod_message_destroy(ipodMessage);
}

bool previous_play_status = false;
void now_playing_state_callback(bool track_data) {
    if(!is_shown){
        return;
    }

    if(track_data) {
        marquee_text_layer_set_text(now_playing_album_layer, ipod_get_album());
        marquee_text_layer_set_text(now_playing_artist_layer, ipod_get_artist());
        marquee_text_layer_set_text(now_playing_title_layer, ipod_get_title());
    }
    else {
        if(previous_play_status != now_playing_is_playing_music()){
            now_playing_action_bar_handle(false);
        }

        action_bar_layer_set_icon(control_action_bar, BUTTON_ID_SELECT,
            now_playing_is_playing_music() ? icon_pause : icon_play);
        progress_bar_layer_set_range(now_playing_progress_bar, 0, ipod_state_duration());
        progress_bar_layer_set_value(now_playing_progress_bar, ipod_state_current_time());

        previous_play_status = now_playing_is_playing_music();
    }
}

void now_playing_set_album_art(GBitmap *album_art){
    if(album_art == NULL){
        display_no_album();
        return;
    }

    now_playing_album_art_bitmap = album_art;
    if(now_playing_album_art_layer){
        NSLog("Setting album art onto layer.");
        bitmap_layer_set_bitmap(now_playing_album_art_layer, now_playing_album_art_bitmap);
    }
}

void now_playing_tick() {
    static uint8_t current_jump = 0;
    if(now_playing_progress_bar){
        if(!now_playing_settings.battery_saver || (now_playing_settings.battery_saver && current_jump == 5)){
            progress_bar_layer_set_value(now_playing_progress_bar, ipod_state_current_time());
            current_jump = 0;
        }
        current_jump++;
    }
}

void now_playing_animation_tick() {
    if(!is_shown || now_playing_settings.battery_saver) {
        return;
    }

    now_playing_timer = app_timer_register(33, ipod_app_timer_handler, NULL);
}

void now_playing_graphics_proc(Layer *layer, GContext *ctx){
    graphics_context_set_fill_color(ctx, background_colour);
    GSize artist_text_size = graphics_text_layout_get_content_size(ipod_get_artist(), artist_font, GRect(0, 120, 144, 18), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
    uint8_t padding = 4;
    graphics_fill_rect(ctx, GRect((144/2)-(artist_text_size.w/2)-padding, 120, artist_text_size.w+(padding*2), 18), 2, GCornersAll);
}

bool now_playing_action_bar_is_showing = true;

void animate_action_bar(void *action_bar_pointer){
    ActionBarLayer *action_bar = (ActionBarLayer*)action_bar_pointer;

    Layer *action_bar_layer = action_bar_layer_get_layer(action_bar);

    GRect current_frame = layer_get_frame(action_bar_layer);
    GRect next_frame;

    next_frame = GRect(current_frame.origin.x + (now_playing_action_bar_is_showing ? (ACTION_BAR_WIDTH) : (-(ACTION_BAR_WIDTH))), current_frame.origin.y, current_frame.size.w, current_frame.size.h);

    animate_layer(action_bar_layer, &current_frame, &next_frame, now_playing_settings.battery_saver ? 50 : 400, 0);

    now_playing_action_bar_is_showing = !now_playing_action_bar_is_showing;
}

bool action_bar_timer_registered = false;
void now_playing_action_bar_handle(bool is_select){
    if(action_bar_timer_registered){
        NSWarn("Destroying timer.");
        app_timer_cancel(action_bar_timer);
        action_bar_timer_registered = false;
    }
    if(!now_playing_action_bar_is_showing){
        NSWarn("Animating actionbar.");
        animate_action_bar(control_action_bar);
        action_bar_timer_registered = false;
    }
    if(now_playing_is_playing_music() && !is_select){
        NSWarn("Registering timer.");
        action_bar_timer = app_timer_register(2000, animate_action_bar, control_action_bar);
        action_bar_timer_registered = true;
    }
}

void now_playing_clicked_up(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        now_playing_send_state_change(NowPlayingStateSkipPrevious);
    }
    else {
        now_playing_send_state_change(NowPlayingStateVolumeUp);
    }
    now_playing_action_bar_handle(false);
}

void now_playing_clicked_select(ClickRecognizerRef recognizer, void *context) {
    now_playing_send_state_change(NowPlayingStatePlayPause);

    now_playing_action_bar_handle(true);
}

void now_playing_clicked_down(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        now_playing_send_state_change(NowPlayingStateSkipNext);
    }
    else {
        now_playing_send_state_change(NowPlayingStateVolumeDown);
    }
    now_playing_action_bar_handle(false);
}

void now_playing_long_clicked_select(ClickRecognizerRef recognizer, void *context) {
    controlling_volume = !controlling_volume;
    if(controlling_volume) {
        action_bar_layer_set_icon(control_action_bar, BUTTON_ID_UP, icon_volume_up);
        action_bar_layer_set_icon(control_action_bar, BUTTON_ID_DOWN, icon_volume_down);
    }
    else {
        action_bar_layer_set_icon(control_action_bar, BUTTON_ID_DOWN, icon_fast_forward);
        action_bar_layer_set_icon(control_action_bar, BUTTON_ID_UP, icon_rewind);
    }

    static const uint32_t const segments[] = { 50, 0 };
    VibePattern pattern = {
        .durations = segments,
        .num_segments = ARRAY_LENGTH(segments),
    };
    vibes_enqueue_custom_pattern(pattern);

    now_playing_action_bar_handle(false);
}

void now_playing_click_config_provider(void* context) {
    window_single_click_subscribe(BUTTON_ID_DOWN, now_playing_clicked_down);
    window_single_click_subscribe(BUTTON_ID_UP, now_playing_clicked_up);
    window_single_click_subscribe(BUTTON_ID_SELECT, now_playing_clicked_select);

    window_long_click_subscribe(BUTTON_ID_SELECT, 500, now_playing_long_clicked_select, NULL);
}

void now_playing_window_load(Window* window) {
    Layer *window_layer = window_get_root_layer(window);

    now_playing_settings = settings_get_settings();

    icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
    icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
    icon_fast_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_FAST_FORWARD);
    icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_REWIND);
    icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
    icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);

    no_album_art_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NO_ALBUM_ART);

    now_playing_album_art_layer = bitmap_layer_create(GRect(0, 0, 144, 144));
    if(now_playing_album_art_bitmap){
        bitmap_layer_set_bitmap(now_playing_album_art_layer, now_playing_album_art_bitmap);
    }
    else{
        display_no_album();
    }
    layer_add_child(window_layer, bitmap_layer_get_layer(now_playing_album_art_layer));

    now_playing_graphics_layer = layer_create(GRect(0, 0, 144, 168));
    layer_set_update_proc(now_playing_graphics_layer, now_playing_graphics_proc);
    layer_add_child(window_layer, now_playing_graphics_layer);

    now_playing_title_layer = marquee_text_layer_create(GRect(2, 142, 140, 28));
    marquee_text_layer_set_font(now_playing_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(now_playing_title_layer, ipod_get_title());
    layer_add_child(window_layer, marquee_text_layer_get_layer(now_playing_title_layer));

    now_playing_album_layer = marquee_text_layer_create(GRect(2, 130, 112, 23));
    marquee_text_layer_set_font(now_playing_album_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(now_playing_album_layer, ipod_get_album());
    //layer_add_child(window_layer, marquee_text_layer_get_layer(now_playing_title_layer));

    artist_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    now_playing_artist_layer = marquee_text_layer_create(GRect(22, 119, 100, 24));
    marquee_text_layer_set_font(now_playing_artist_layer, artist_font);
    marquee_text_layer_set_text(now_playing_artist_layer, ipod_get_artist());
    layer_add_child(window_layer, marquee_text_layer_get_layer(now_playing_artist_layer));

    now_playing_progress_bar = progress_bar_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(window_layer, progress_bar_layer_get_layer(now_playing_progress_bar));

    control_action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(control_action_bar, window);
    action_bar_layer_set_click_config_provider(control_action_bar, now_playing_click_config_provider);
    action_bar_layer_set_icon(control_action_bar, BUTTON_ID_DOWN, icon_fast_forward);
    action_bar_layer_set_icon(control_action_bar, BUTTON_ID_UP, icon_rewind);
    action_bar_layer_set_icon(control_action_bar, BUTTON_ID_SELECT, icon_play);
    controlling_volume = false;
    previous_play_status = now_playing_is_playing_music();

    ipod_state_set_callback(now_playing_state_callback);

    is_shown = true;
    now_playing_state_callback(false);
    now_playing_animation_tick();
    now_playing_action_bar_handle(false);
}

void now_playing_window_unload(Window* window) {
    action_bar_layer_destroy(control_action_bar);

    marquee_text_layer_destroy(now_playing_title_layer);
    marquee_text_layer_destroy(now_playing_album_layer);
    marquee_text_layer_destroy(now_playing_artist_layer);

    progress_bar_layer_destroy(now_playing_progress_bar);

    gbitmap_destroy(icon_pause);
    gbitmap_destroy(icon_play);
    gbitmap_destroy(icon_fast_forward);
    gbitmap_destroy(icon_rewind);
    gbitmap_destroy(icon_volume_up);
    gbitmap_destroy(icon_volume_down);

    ipod_state_set_callback(NULL);
    is_shown = false;
}

void now_playing_show() {
    background_colour = GColorBlack;

    now_playing_window = window_create();
    window_set_background_color(now_playing_window, background_colour);
    window_set_window_handlers(now_playing_window, (WindowHandlers){
        .unload = now_playing_window_unload,
        .load = now_playing_window_load,
    });

    window_stack_push(now_playing_window, true);
}