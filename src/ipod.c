#include <pebble.h>

#include "main_menu.h"
#include "library_menus.h"
#include "marquee_text.h"
#include "ipod_state.h"
#include "now_playing.h"

Window *window;

void ipod_init() {
    //g_app_context = ctx;

    window = window_create();
    window_stack_push(window, true);

    main_menu_init(window);
    init_library_menus();
    ipod_state_init();

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
	NSLog("first result %d", app_message_outbox_begin(&iter));

	//app_timer_register(1800000, update_weather, NULL);

	if (iter == NULL) {
		APP_LOG(APP_LOG_LEVEL_ERROR, "Iter is NULL, refusing to send message.");
		return;
	}

	dict_write_uint16(iter, 200, 1);
	dict_write_end(iter);

	NSLog("result %d", app_message_outbox_send());
}

static void received_handler(DictionaryIterator *iter, void *context) {
    NSLog("Got shit wake message");
}

int main() {
    //NSLog("Opening inbox, %d", app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum()));
    app_message_open(2048, 2048);
    app_message_register_inbox_dropped(dropped_inbox);
    app_message_register_outbox_failed(failed_outbox);
    app_message_register_inbox_received(received_handler);

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    send_test_message();

    ipod_init();
    app_event_loop();
}
