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

int main() {
    app_message_open(124, 256);
    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
    ipod_init();
    app_event_loop();
}
