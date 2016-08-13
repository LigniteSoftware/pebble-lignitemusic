#include <lignite_music.h>

Window *window;

void ipod_create() {
    NSLog("Before load %d", heap_bytes_free());
    window = window_create();
    window_stack_push(window, true);
    NSLog("After load %d", heap_bytes_free());

    main_menu_create(window);
    library_menus_create();
    ipod_state_create();
}

void ipod_app_timer_handler() {
    marquee_text_layer_tick();
    now_playing_animation_tick();
}

void tick_handler(struct tm *t, TimeUnits units) {
    ipod_state_tick();
    now_playing_tick();
}

void dropped_inbox(AppMessageResult reason, void *context){
    NSWarn("Dropped inbox, reason %d", reason);
}

void auto_destroy_error_window(void *window){
    message_window_pop_off_window((MessageWindow*)window, true, true);
}

void failed_outbox(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    NSWarn("Failed outbox, reason %d", reason);

    MessageWindow *failed_outbox = message_window_create();
    static char outbox_buffer[65];
    snprintf(outbox_buffer, sizeof(outbox_buffer), "Error %d. Make sure the phone app is open too and try again.", reason);
    message_window_set_text(failed_outbox, outbox_buffer);
    message_window_push_on_window(failed_outbox, window_stack_get_top_window(), true);

    vibes_long_pulse();

    app_timer_register(10000, auto_destroy_error_window, failed_outbox);
}

void pebblekit_connection_handler(bool connected){
    NSWarn("Got PebbleKit connection: %d", connected);
}

void pebble_app_connection_handler(bool connected){
    NSWarn("Got Pebble app connection: %d", connected);
    if(connected){
        vibes_long_pulse();
    }
    else{
        vibes_short_pulse();
    }
}

int main() {
    NSWarn("Before main %d", heap_bytes_free());
    settings_load();

    app_message_register_inbox_dropped(dropped_inbox);
    app_message_register_outbox_failed(failed_outbox);
    app_message_open(APP_MESSAGE_SIZE, APP_MESSAGE_SIZE);

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    connection_service_subscribe((ConnectionHandlers){
        .pebble_app_connection_handler = pebble_app_connection_handler,
        .pebblekit_connection_handler = pebblekit_connection_handler
    });

    ipod_create();
    NSWarn("After main %d", heap_bytes_free());
    app_event_loop();

    settings_save();
}
