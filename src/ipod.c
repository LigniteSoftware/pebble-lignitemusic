#include <lignite_music.h>

Window *window;

void ipod_create() {
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

int main() {
    settings_load();

    connection_window_attach();

    app_message_open(APP_MESSAGE_SIZE, APP_MESSAGE_SIZE/4);

    tick_timer_service_subscribe(SECOND_UNIT, tick_handler);

    ipod_create();
    app_event_loop();

    settings_save();
}
