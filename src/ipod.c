#include <lignite_music.h>

Window *window;

void ipod_init() {
    window = window_create();
    window_stack_push(window, true);

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
    NSLog("Dropped inbox, reason %d", reason);
}

void auto_destroy_error_window(void *window){
    MessageWindow *message_window = (MessageWindow*)window;
    message_window_destroy(message_window);
}

void failed_outbox(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    NSLog("Failed outbox, reason %d", reason);

    MessageWindow *failed_outbox = message_window_create();
    static char outbox_buffer[65];
    snprintf(outbox_buffer, sizeof(outbox_buffer), "Error B-%d. This usually means the app isn't open on your phone.", reason);
    message_window_set_text(failed_outbox, outbox_buffer);
    message_window_push_on_window(failed_outbox, window_stack_get_top_window(), true);

    vibes_long_pulse();

    app_timer_register(5000, auto_destroy_error_window, failed_outbox);
}

void send_test_message(){
    DictionaryIterator *iter;
	NSLog("Begin test message, outbox begin result of %d", app_message_outbox_begin(&iter));

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is NULL, refusing to send message.");
		return;
	}

	dict_write_uint16(iter, 200, 1);
	dict_write_end(iter);

	NSLog("Send result of %d", app_message_outbox_send());
}

int main() {
    settings_load();

    app_message_register_inbox_dropped(dropped_inbox);
    app_message_register_outbox_failed(failed_outbox);
    NSLog("Bytes free before %d", heap_bytes_free());
    app_message_open(1600, 1600);
    NSLog("Bytes free after %d", heap_bytes_free());

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    ipod_init();
    app_event_loop();

    settings_save();
}
