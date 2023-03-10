#include <lignite_music.h>

void now_playing_action_bar_handle(bool is_select);

Window *now_playing_window;

ActionBarLayer *control_action_bar;
BitmapLayer *now_playing_album_art_layer[IMAGE_PARTS];
Layer *now_playing_graphics_layer;
MarqueeTextLayer *now_playing_title_layer, *now_playing_artist_layer;
ProgressBarLayer *now_playing_progress_bar;

GBitmap *now_playing_album_art_bitmap[IMAGE_PARTS], *no_album_art_bitmap = NULL;

AppTimer *now_playing_timer, *action_bar_timer, *invert_controls_timer;
GColor background_colour;
GFont artist_font;

Settings now_playing_settings;

static bool controlling_volume = false, is_shown = false, draw_no_album = false;
static GBitmap *icon_pause, *icon_play, *icon_fast_forward, *icon_rewind, *icon_volume_up, *icon_volume_down, *icon_more;

GRect artist_frame;

bool pebble_controls_pushed_select = false;

/*
 * When there is no album art available (either still loading or the phone has said so), call this
 * to display a basic coverart
 */
void display_no_album(uint8_t image_part) {
    if(now_playing_album_art_layer[image_part]){
        bitmap_layer_set_bitmap(now_playing_album_art_layer[image_part], NULL);
    }

    draw_no_album = true;
    if(now_playing_graphics_layer){
        layer_mark_dirty(now_playing_graphics_layer);
    }
}

bool now_playing_is_playing_music(){
    return ipod_get_playback_state() == MPMusicPlaybackStatePlaying;
}

void now_playing_check_for_no_music(){
    if(strcmp(ipod_get_title(), "") == 0){
        strncpy(ipod_get_artist(), "No Music", 9);
        strncpy(ipod_get_title(), "Start music on your phone or watch", 35);
    }
}

void now_playing_send_state_change(NowPlayingState state_change) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        return;
    }

    dict_write_int8(ipodMessage->iter, MessageKeyChangeState, state_change);
    app_message_outbox_send();
}

bool previous_play_status = false;
void now_playing_state_callback(bool track_data) {
    if(!is_shown){
        return;
    }

    if(track_data) {
        now_playing_check_for_no_music();
        marquee_text_layer_set_text(now_playing_artist_layer, ipod_get_artist());
        marquee_text_layer_set_text(now_playing_title_layer, ipod_get_title());
    }
    else {
        if(previous_play_status != now_playing_is_playing_music()){
            now_playing_action_bar_handle(false);
        }

        if(now_playing_settings.pebble_controls){
            if(controlling_volume){
                action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT,
                    now_playing_is_playing_music() ? icon_pause : icon_play, true);
            }
            else{
                action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT,
                    now_playing_is_playing_music() ? icon_more : icon_play, true);
                    pebble_controls_pushed_select = false;
            }
        }
        else{
        action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT,
            now_playing_is_playing_music() ? icon_pause : icon_play, true);
        }
        progress_bar_layer_set_range(now_playing_progress_bar, 0, ipod_state_duration());
        progress_bar_layer_set_value(now_playing_progress_bar, ipod_state_current_time());

        previous_play_status = now_playing_is_playing_music();
    }
}

void now_playing_set_album_art(uint8_t image_part, GBitmap *album_art){
    now_playing_album_art_bitmap[image_part] = album_art;

    if(draw_no_album && image_part == IMAGE_PARTS-1){
        draw_no_album = false;
        if(now_playing_graphics_layer){
            layer_mark_dirty(now_playing_graphics_layer);
        }
    }

    if(!now_playing_album_art_layer[image_part]){
        return;
    }
    if(album_art == NULL){
        display_no_album(image_part);
        return;
    }

    if(now_playing_album_art_layer[image_part]){
        bitmap_layer_set_bitmap(now_playing_album_art_layer[image_part], now_playing_album_art_bitmap[image_part]);
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
    #ifdef PBL_ROUND
    GRect title_frame = GRect(0, 135, 180, 45);
    #else
    GRect title_frame = GRect(0, 144, 144, 40);
    #endif

    if(draw_no_album){
        graphics_context_set_fill_color(ctx, GColorWhite);
        for(uint8_t i = 0; i < IMAGE_PARTS; i++){
            graphics_fill_rect(ctx, layer_get_bounds(bitmap_layer_get_layer(now_playing_album_art_layer[i])), 0, GCornerNone);
        }
        graphics_context_set_compositing_mode(ctx, GCompOpSet);
        GSize bitmap_size = gbitmap_get_bounds(no_album_art_bitmap).size;
        GRect image_frame = layer_get_bounds(bitmap_layer_get_layer(now_playing_album_art_layer[0]));
        GRect now_playing_frame = GRect((title_frame.size.w/2)-(bitmap_size.w/2),
            (image_frame.size.h/2)-(bitmap_size.h/2), bitmap_size.w, bitmap_size.h);
        graphics_draw_bitmap_in_rect(ctx, no_album_art_bitmap, now_playing_frame);
    }

    graphics_context_set_fill_color(ctx, background_colour);
    graphics_fill_rect(ctx, title_frame, 0, GCornerNone);

    if(now_playing_settings.artist_label){
        GSize artist_text_size = graphics_text_layout_get_content_size(ipod_get_artist(), artist_font, artist_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
        uint8_t padding = 4;
        if(artist_text_size.w > 0){
            int x = (WINDOW_FRAME.size.w/2)-(artist_text_size.w/2)-padding;
            int width = artist_text_size.w+(padding*2);
            if(x < 22){
                x = 22-padding;
                width = (WINDOW_FRAME.size.w-44)+(padding*2);
            }
            graphics_fill_rect(ctx, GRect(x, artist_frame.origin.y+1, width, 18), 2, GCornersAll);
        }
    }

    if(now_playing_settings.show_time){
        graphics_context_set_fill_color(ctx, GColorBlack);

        GRect bar_frame = GRect(0, 0, WINDOW_FRAME.size.w, PBL_IF_ROUND_ELSE(10, 6));
        graphics_fill_rect(ctx, bar_frame, 0, GCornerNone);

        uint8_t time_width = 40;
        GRect time_frame = GRect((WINDOW_FRAME.size.w/2)-(time_width/2), PBL_IF_ROUND_ELSE(4, 0), time_width, 18);
        graphics_fill_rect(ctx, time_frame, 2, GCornersAll);

        struct tm *t;
      	time_t temp;
      	temp = time(NULL);
      	t = localtime(&temp);

        static char time_buffer[] = "00:00";
        strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", t);

        graphics_context_set_text_color(ctx, GColorWhite);
        graphics_draw_text(ctx, time_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_14), time_frame, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

        #ifndef PBL_PLATFORM_APLITE
        #ifndef PBL_PLATFORM_CHALK
        graphics_draw_pixel(ctx, GPoint(0, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(1, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(0, bar_frame.size.h+1));

        graphics_draw_pixel(ctx, GPoint(bar_frame.size.w-1, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(bar_frame.size.w-2, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(bar_frame.size.w-1, bar_frame.size.h+1));
        #endif

        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x-1, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x-2, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x-1, bar_frame.size.h+1));

        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x+time_frame.size.w, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x+time_frame.size.w+1, bar_frame.size.h));
        graphics_draw_pixel(ctx, GPoint(time_frame.origin.x+time_frame.size.w, bar_frame.size.h+1));
        #endif
    }

    #ifndef PBL_PLATFORM_APLITE
    #ifndef PBL_PLATFORM_CHALK
    graphics_draw_pixel(ctx, GPoint(0, title_frame.origin.y-1));
    graphics_draw_pixel(ctx, GPoint(1, title_frame.origin.y-1));
    graphics_draw_pixel(ctx, GPoint(0, title_frame.origin.y-2));

    graphics_draw_pixel(ctx, GPoint(title_frame.size.w-1, title_frame.origin.y-1));
    graphics_draw_pixel(ctx, GPoint(title_frame.size.w-2, title_frame.origin.y-1));
    graphics_draw_pixel(ctx, GPoint(title_frame.size.w-1, title_frame.origin.y-2));
    #endif
    #endif
}

bool now_playing_action_bar_is_showing = true;
#ifndef PBL_PLATFORM_APLITE
uint16_t action_bar_showing = 0, action_bar_hidden = 0;
#endif

void animate_action_bar(void *action_bar_pointer){
    ActionBarLayer *action_bar = (ActionBarLayer*)action_bar_pointer;
    Layer *action_bar_layer = action_bar_layer_get_layer(action_bar);

    #ifndef PBL_PLATFORM_APLITE
    GRect current_frame = layer_get_frame(action_bar_layer);
    GRect next_frame;

    next_frame = GRect(now_playing_action_bar_is_showing ? action_bar_hidden : action_bar_showing, current_frame.origin.y, current_frame.size.w, current_frame.size.h);

    animate_layer(action_bar_layer, &current_frame, &next_frame, now_playing_settings.battery_saver ? 50 : 400, 0);
    #else
    layer_set_hidden(action_bar_layer, now_playing_action_bar_is_showing);
    #endif

    now_playing_action_bar_is_showing = !now_playing_action_bar_is_showing;
}

bool action_bar_timer_registered = false;
void now_playing_action_bar_handle(bool is_select){
    if(!now_playing_action_bar_is_showing){
        animate_action_bar(control_action_bar);
        action_bar_timer_registered = false;
    }
    if(action_bar_timer_registered){
        app_timer_cancel(action_bar_timer);
        action_bar_timer_registered = false;
    }
    if(now_playing_is_playing_music() && (!is_select || now_playing_settings.pebble_controls)){
        action_bar_timer = app_timer_register(now_playing_settings.pebble_controls ? 3500 : 2000, animate_action_bar, control_action_bar);
        action_bar_timer_registered = true;
    }
}

void now_playing_invert_button_controls(){
    controlling_volume = !controlling_volume;
    if(controlling_volume) {
        action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_UP, icon_volume_up, true);
        action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_DOWN, icon_volume_down, true);
    }
    else {
        action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_DOWN, icon_fast_forward, true);
        action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_UP, icon_rewind, true);
    }
    if(now_playing_settings.pebble_controls){
        if(now_playing_is_playing_music()){
            action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT, controlling_volume ? icon_pause : icon_more, true);
            pebble_controls_pushed_select = false;
        }
        invert_controls_timer = NULL;
    }
}

void now_playing_reset_invert_controls(){
    if(!now_playing_settings.pebble_controls){
        return;
    }
    if(invert_controls_timer){
        app_timer_cancel(invert_controls_timer);
        invert_controls_timer = NULL;
    }
    invert_controls_timer = app_timer_register(3000, now_playing_invert_button_controls, NULL);
}

void now_playing_clicked_up(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        now_playing_send_state_change(NowPlayingStateSkipPrevious);
    }
    else {
        now_playing_send_state_change(NowPlayingStateVolumeUp);
        now_playing_reset_invert_controls();
    }
    now_playing_action_bar_handle(false);
}

void now_playing_clicked_select(ClickRecognizerRef recognizer, void *context) {
    if(now_playing_settings.pebble_controls){
        if(!now_playing_is_playing_music()){
            now_playing_send_state_change(NowPlayingStatePlayPause);
        }
        else if(!pebble_controls_pushed_select){
            now_playing_invert_button_controls();
            now_playing_reset_invert_controls();
        }
        else{
            now_playing_send_state_change(NowPlayingStatePlayPause);
        }
        pebble_controls_pushed_select = true;
    }
    else{
        now_playing_send_state_change(NowPlayingStatePlayPause);
    }
    now_playing_action_bar_handle(true);
}

void now_playing_clicked_down(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        now_playing_send_state_change(NowPlayingStateSkipNext);
    }
    else {
        now_playing_send_state_change(NowPlayingStateVolumeDown);
        now_playing_reset_invert_controls();
    }
    now_playing_action_bar_handle(false);
}

void now_playing_long_clicked_select(ClickRecognizerRef recognizer, void *context) {
    if(now_playing_settings.pebble_controls){
        return;
    }

    now_playing_invert_button_controls();

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

void now_playing_new_settings(Settings new_settings){
    if(!is_shown){
        return;
    }

    if(now_playing_settings.battery_saver != new_settings.battery_saver){
        marquee_text_layer_mark_dirty(now_playing_artist_layer);
        marquee_text_layer_mark_dirty(now_playing_title_layer);
    }

    now_playing_settings = new_settings;

    layer_set_hidden(marquee_text_layer_get_layer(now_playing_artist_layer), !now_playing_settings.artist_label);

    layer_mark_dirty(now_playing_graphics_layer);

    // if(new_settings.pebble_controls){
    //     // action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT, now_playing_is_playing_music() ? icon_more : icon_play, true);
    //     if(controlling_volume){
    //         NSLog("Inverting from new settings");
    //         //now_playing_invert_button_controls();
    //     }
    // }
    // else{
    //     // action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT, now_playing_is_playing_music() ? icon_pause : icon_play, true);
    // }

    app_timer_cancel(now_playing_timer);
    now_playing_animation_tick();
}

void now_playing_window_load(Window* window) {
    Layer *window_layer = window_get_root_layer(window);

    #ifdef PBL_PLATFORM_APLITE
    main_menu_destroy();
    #endif

    library_menus_pop_all();
    now_playing_settings = settings_get_settings();

    icon_pause = gbitmap_create_with_resource(RESOURCE_ID_ICON_PAUSE);
    icon_play = gbitmap_create_with_resource(RESOURCE_ID_ICON_PLAY);
    icon_fast_forward = gbitmap_create_with_resource(RESOURCE_ID_ICON_NEXT_SONG);
    icon_rewind = gbitmap_create_with_resource(RESOURCE_ID_ICON_LAST_SONG);
    icon_volume_up = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_UP);
    icon_volume_down = gbitmap_create_with_resource(RESOURCE_ID_ICON_VOLUME_DOWN);
    icon_more = gbitmap_create_with_resource(RESOURCE_ID_ICON_MORE);

    no_album_art_bitmap = gbitmap_create_with_resource(RESOURCE_ID_NO_ALBUM_ART);

    for(uint8_t i = 0; i < IMAGE_PARTS; i++){
        uint16_t image_width = WINDOW_FRAME.size.w/IMAGE_PARTS;
        GRect image_frame = GRect(image_width*i, 0, image_width, WINDOW_FRAME.size.h-PBL_IF_ROUND_ELSE(45, 168-144));
        now_playing_album_art_layer[i] = bitmap_layer_create(image_frame);
        if(now_playing_album_art_bitmap[i]){
            bitmap_layer_set_bitmap(now_playing_album_art_layer[i], now_playing_album_art_bitmap[i]);
        }
        else{
            display_no_album(i);
        }
        layer_add_child(window_layer, bitmap_layer_get_layer(now_playing_album_art_layer[i]));
    }

    #ifdef PBL_ROUND
    uint8_t title_offset = 30;
    GRect title_frame = GRect(title_offset, 132, 180-(title_offset*2), 28);
    artist_frame = GRect(22, 114, 136, 24);
    #else
    GRect title_frame = GRect(2, 142, 140, 28);
    artist_frame = GRect(22, 119, 100, 24);
    #endif

    now_playing_graphics_layer = layer_create(WINDOW_FRAME);
    layer_set_update_proc(now_playing_graphics_layer, now_playing_graphics_proc);
    layer_add_child(window_layer, now_playing_graphics_layer);

    now_playing_title_layer = marquee_text_layer_create(title_frame);
    marquee_text_layer_set_font(now_playing_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(now_playing_title_layer, ipod_get_title());
    layer_add_child(window_layer, marquee_text_layer_get_layer(now_playing_title_layer));

    artist_font = fonts_get_system_font(FONT_KEY_GOTHIC_14);
    now_playing_artist_layer = marquee_text_layer_create(artist_frame);
    marquee_text_layer_set_font(now_playing_artist_layer, artist_font);
    marquee_text_layer_set_text(now_playing_artist_layer, ipod_get_artist());
    layer_add_child(window_layer, marquee_text_layer_get_layer(now_playing_artist_layer));

    now_playing_check_for_no_music();

    now_playing_progress_bar = progress_bar_layer_create(GRect(0, 0, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h));
    layer_add_child(window_layer, progress_bar_layer_get_layer(now_playing_progress_bar));

    control_action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(control_action_bar, window);
    action_bar_layer_set_click_config_provider(control_action_bar, now_playing_click_config_provider);
    action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_DOWN, icon_fast_forward, false);
    action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_UP, icon_rewind, false);
    action_bar_layer_set_icon_animated(control_action_bar, BUTTON_ID_SELECT, icon_play, false);
    controlling_volume = false;
    now_playing_action_bar_is_showing = true;
    #ifndef PBL_PLATFORM_APLITE
    GRect current_actionbar_frame = layer_get_frame(action_bar_layer_get_layer(control_action_bar));
    action_bar_showing = current_actionbar_frame.origin.x;
    action_bar_hidden = action_bar_showing+ACTION_BAR_WIDTH;
    #endif

    previous_play_status = now_playing_is_playing_music();

    ipod_state_set_callback(now_playing_state_callback);

    is_shown = true;

    now_playing_state_callback(false);
    now_playing_action_bar_handle(false);

    now_playing_new_settings(settings_get_settings());

    settings_service_subscribe(now_playing_new_settings);

    now_playing_request(NowPlayingRequestTypeOnlyAlbumArt);
}

void now_playing_window_unload(Window* window) {
    is_shown = false;

    action_bar_layer_destroy(control_action_bar);

    if(invert_controls_timer){
        app_timer_cancel(invert_controls_timer);
    }

    marquee_text_layer_destroy(now_playing_title_layer);
    marquee_text_layer_destroy(now_playing_artist_layer);

    progress_bar_layer_destroy(now_playing_progress_bar);
    now_playing_progress_bar = NULL;

    layer_destroy(now_playing_graphics_layer);
    now_playing_graphics_layer = NULL;

    if(no_album_art_bitmap){
        gbitmap_destroy(no_album_art_bitmap);
        no_album_art_bitmap = NULL;
    }
    gbitmap_destroy(icon_pause);
    gbitmap_destroy(icon_play);
    gbitmap_destroy(icon_fast_forward);
    gbitmap_destroy(icon_rewind);
    gbitmap_destroy(icon_volume_up);
    gbitmap_destroy(icon_volume_down);
    gbitmap_destroy(icon_more);

    for(uint8_t i = 0; i < IMAGE_PARTS; i++){
        bitmap_layer_destroy(now_playing_album_art_layer[i]);
        now_playing_album_art_layer[i] = NULL;
    }

    window_destroy(now_playing_window);

    #ifdef PBL_PLATFORM_APLITE
    main_menu_create(NULL);
    #endif
}

void now_playing_window_appear(Window *window){
    connection_window_set_wakeup_cookie(WakeupCookieNowPlaying);
}

void now_playing_show() {
    if(is_shown){
        return;
    }
    background_colour = GColorBlack;

    now_playing_window = window_create();
    window_set_background_color(now_playing_window, GColorWhite);
    window_set_window_handlers(now_playing_window, (WindowHandlers){
        .unload = now_playing_window_unload,
        .load = now_playing_window_load,
        .appear = now_playing_window_appear
    });

    window_stack_push(now_playing_window, true);
}

bool now_playing_is_shown(){
    return is_shown;
}