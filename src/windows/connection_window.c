#include <lignite_music.h>

ConnectionWindow *connection_window;

void connection_window_click_select(ClickRecognizerRef ref, void *context){
    const time_t future_timestamp = time(NULL) + 1;

    // Choose a 'cookie' value representing the reason for the wakeup
    const int cookie = 0;

    // If true, the user will be notified if they missed the wakeup
    // (i.e. their watch was off)
    const bool notify_if_missed = true;

    // Schedule wakeup event
    WakeupId id = wakeup_schedule(future_timestamp, cookie, notify_if_missed);

    // Check the scheduling was successful
    if(id >= 0) {
        // Persist the ID so that a future launch can query it
        const int wakeup_id_key = 43;
        persist_write_int(wakeup_id_key, id);
        window_stack_pop_all(true);
    }
    NSLog("%d", (int)id);
}

void connection_window_click_back(ClickRecognizerRef ref, void *context){
    window_stack_pop_all(true);
}

void connection_window_click_config_provider(void *context){
    window_single_click_subscribe(BUTTON_ID_BACK, connection_window_click_back);
    window_single_click_subscribe(BUTTON_ID_SELECT, connection_window_click_select);
}

void connection_window_background_proc(Layer *layer, GContext *ctx){
    GRect frame = layer_get_frame(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, frame, 0, GCornerNone);
}

void connection_window_load(Window *window){
    Layer *window_layer = window_get_root_layer(window);

    static char title[30], description[150];

    NSLog("%d", connection_window->error);
    switch(connection_window->error){
        case ConnectionErrorPebbleAppDisconnected:
            snprintf(title, sizeof(title), "Pebble App Not Connected");
            snprintf(description, sizeof(description), "The Pebble app isn't connected. Please make sure your watch is connected and Pebble app open in the background. This will disappear when reconnected.");
            break;
        case ConnectionErrorPebbleKitDisconnected:
            snprintf(title, sizeof(title), "Lignite App Not Connected");
            snprintf(description, sizeof(description), "The Lignite app isn't connected. Please make sure the Lignite app is open in the background. This will disappear when reconnected.");
            break;
        case ConnectionErrorOutboxDropped:
            snprintf(title, sizeof(title), "Failed to Send");
            snprintf(description, sizeof(description), "The watchapp is having a tough time getting messages to the phone, error code X. Press select to retry and contact us if this issue continues.");
            break;
        case ConnectionErrorAppMessageBusy: //Worst error in the world
            snprintf(title, sizeof(title), "Pebble Internal Error");
            snprintf(description, sizeof(description), "Pebble has an annoying bug where a watchapp's communication will lock up. Please press the select button to reboot the watchapp to fix this issue.");
            break;
        case ConnectionErrorOther:
            snprintf(title, sizeof(title), "Other Error");
            snprintf(description, sizeof(description), "There was an unknown error while preforming this operation. Please try pressing the select button to reboot the app and try again.");
            break;
    }

    GRect title_layer_frame = GRect(0, PBL_IF_ROUND_ELSE(10, 0), WINDOW_FRAME.size.w, 2000);
    GSize title_size = graphics_text_layout_get_content_size(title, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), title_layer_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
    title_layer_frame = GRect(0, PBL_IF_ROUND_ELSE(10, 0), WINDOW_FRAME.size.w, title_size.h);

    connection_window->title_layer = text_layer_create(title_layer_frame);
    text_layer_set_text(connection_window->title_layer, title);
    text_layer_set_font(connection_window->title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_color(connection_window->title_layer, GColorWhite);
    text_layer_set_background_color(connection_window->title_layer, GColorBlack);

    GRect description_layer_frame = GRect(0, title_layer_frame.origin.y+title_layer_frame.size.h+4, WINDOW_FRAME.size.w, 2000);
    connection_window->description_layer = text_layer_create(description_layer_frame);
    text_layer_set_text(connection_window->description_layer, description);
    text_layer_set_font(connection_window->description_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_color(connection_window->description_layer, GColorWhite);
    text_layer_set_background_color(connection_window->description_layer, GColorBlack);
    GSize max_size = graphics_text_layout_get_content_size(description, fonts_get_system_font(FONT_KEY_GOTHIC_24), description_layer_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
    max_size.w = WINDOW_FRAME.size.w;
    max_size.h += PBL_IF_ROUND_ELSE(150, 4);
    description_layer_frame.size.h = max_size.h;
    text_layer_set_size(connection_window->description_layer, max_size);

    int content_height = description_layer_frame.origin.y+description_layer_frame.size.h+4;

    connection_window->background_layer = layer_create(GRect(0, 0, WINDOW_FRAME.size.w, content_height));
    layer_set_update_proc(connection_window->background_layer, connection_window_background_proc);

    connection_window->root_layer = scroll_layer_create(WINDOW_FRAME);
    scroll_layer_set_callbacks(connection_window->root_layer, (ScrollLayerCallbacks){
        .click_config_provider = connection_window_click_config_provider
    });
    scroll_layer_set_click_config_onto_window(connection_window->root_layer, window);
    scroll_layer_set_content_size(connection_window->root_layer, GSize(WINDOW_FRAME.size.w, content_height));
    scroll_layer_add_child(connection_window->root_layer, connection_window->background_layer);
    scroll_layer_add_child(connection_window->root_layer, text_layer_get_layer(connection_window->title_layer));
    scroll_layer_add_child(connection_window->root_layer, text_layer_get_layer(connection_window->description_layer));

    layer_add_child(window_layer, scroll_layer_get_layer(connection_window->root_layer));

    #ifdef PBL_ROUND
    text_layer_set_text_alignment(connection_window->description_layer, GTextAlignmentCenter);
    text_layer_enable_screen_text_flow_and_paging(connection_window->description_layer, 8);
    text_layer_set_text_alignment(connection_window->title_layer, GTextAlignmentCenter);
    text_layer_enable_screen_text_flow_and_paging(connection_window->title_layer, 8);
    scroll_layer_set_paging(connection_window->root_layer, true);
    #endif
}

void connection_window_unload(Window *window){

}

void connection_window_push(ConnectionError error){
    if(connection_window){
        NSWarn("Connection window already exists!");
        return;
    }
    connection_window = malloc(sizeof(ConnectionWindow));
    connection_window->error = error;

    connection_window->window = window_create();
    window_set_window_handlers(connection_window->window, (WindowHandlers){
        .load = connection_window_load,
        .unload = connection_window_unload
    });

    switch(error){
        case ConnectionErrorPebbleAppDisconnected:
        case ConnectionErrorPebbleKitDisconnected:
            connection_window->pebble_app_connected = connection_service_peek_pebble_app_connection();
            connection_window->pebblekit_connected = connection_service_peek_pebblekit_connection();
            break;
        default:
            break;
    }

    window_stack_push(connection_window->window, true);
}

void connection_window_inbox_dropped_handler(AppMessageResult reason, void *context){
    NSError("Dropped inbox, reason %d", reason);
}

void connection_window_outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    NSError("Failed outbox, reason %d", reason);
}

void connection_window_pebble_app_connection_handler(bool connected){

}

void connection_window_pebblekit_connection_handler(bool connected){

}

void connection_window_debug_fire(){
    connection_window_push(ConnectionErrorAppMessageBusy);
}

void connection_window_attach(){
    ConnectionHandlers handlers = (ConnectionHandlers){
        .pebble_app_connection_handler = connection_window_pebble_app_connection_handler,
        .pebblekit_connection_handler = connection_window_pebblekit_connection_handler
    };
    connection_service_subscribe(handlers);

    app_message_register_inbox_dropped(connection_window_inbox_dropped_handler);
    app_message_register_outbox_failed(connection_window_outbox_failed_handler);

    app_timer_register(1000, connection_window_debug_fire, NULL);
}

void connection_window_detach(){
    connection_service_unsubscribe();
}