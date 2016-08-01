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

void failed_outbox(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    NSLog("failed outbox, reason %d", reason);
}

void send_test_message(){
    DictionaryIterator *iter;
	NSLog("Begin outbox result %d", app_message_outbox_begin(&iter));

	//app_timer_register(1800000, update_weather, NULL);

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is NULL, refusing to send message.");
		return;
	}

	dict_write_uint16(iter, 200, 1);
	dict_write_end(iter);

	NSLog("Send result %d", app_message_outbox_send());
}

int main() {
    app_message_register_inbox_dropped(dropped_inbox);
    app_message_register_outbox_failed(failed_outbox);
    app_message_open(2048, 2048);
    //NSLog("Opening inbox, %d", app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum()));

    //tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    //send_test_message();

    ipod_init();
    app_event_loop();
}
