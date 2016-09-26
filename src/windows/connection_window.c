#include <lignite_music.h>

ConnectionWindow *connection_window;
WakeupCookie current_tasty_cookie;

const char *connection_titles[] = {
    "Hold on...", "Pebble Disconnected", "Lignite Disconnected", "Failed", "Pebble Internal Error", "Error"
};

const char *connection_descriptions[] = {
    "The watchapp is currently testing its connection... or press select to reboot the watchapp.",
    "The Pebble app isn't connected. Please make sure your watch is connected and Pebble app open in the background. This will disappear when reconnected.",
    "The Lignite app isn't connected. Please make sure the Lignite app is open in the background. This will disappear when reconnected.",
    "",
    "Pebble has a very annoying bug where a watchapp's communication will lock up. Please press select to reboot the watchapp to fix this temporarily. A fix for this is coming in firmware 4.2.",
    ""
};

void connection_window_click_select(ClickRecognizerRef ref, void *context){
    const time_t future_timestamp = time(NULL) + 1;

    WakeupId id = wakeup_schedule(future_timestamp, current_tasty_cookie, false);
    if(id >= 0) {
        window_stack_pop_all(true);
    }
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

    static char title[30], description[200];

    if(connection_window->error != ConnectionErrorNoError){
        snprintf(title, sizeof(title), connection_titles[connection_window->error]);

        if(connection_window->error == ConnectionErrorOutboxDropped && connection_window->error_code == APP_MSG_OK){
            connection_window->error = ConnectionErrorPebbleKitDisconnected;
        }

        switch(connection_window->error){
            case ConnectionErrorOutboxDropped:
                snprintf(description, sizeof(description), "The watchapp is having a tough time getting messages to the phone (error code %d). Press select to restart the watchapp and contact us if this continues.", connection_window->error_code);
                break;
            case ConnectionErrorOther:
                snprintf(description, sizeof(description), "Spooky! There was an unknown error while preforming this operation (error code %d). Please try pressing the select button to reboot the app and try again.", connection_window->error_code);
                break;
            default:
                snprintf(description, sizeof(description), connection_descriptions[connection_window->error]);
                break;
        }
    }

    GRect title_layer_frame = GRect(0, PBL_IF_ROUND_ELSE(10, 0), WINDOW_FRAME.size.w, 250);
    GSize title_size = graphics_text_layout_get_content_size(title, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD), title_layer_frame, GTextOverflowModeWordWrap, GTextAlignmentCenter);
    title_layer_frame = GRect(0, PBL_IF_ROUND_ELSE(10, 0), WINDOW_FRAME.size.w, title_size.h);

    connection_window->title_layer = text_layer_create(title_layer_frame);
    text_layer_set_text(connection_window->title_layer, title);
    text_layer_set_font(connection_window->title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_text_color(connection_window->title_layer, GColorWhite);
    text_layer_set_overflow_mode(connection_window->title_layer, GTextOverflowModeWordWrap);
    text_layer_set_background_color(connection_window->title_layer, GColorBlack);

    GRect description_layer_frame = GRect(0, title_layer_frame.origin.y+title_layer_frame.size.h+4, WINDOW_FRAME.size.w, 2000);
    connection_window->description_layer = text_layer_create(description_layer_frame);
    text_layer_set_text(connection_window->description_layer, description);
    text_layer_set_font(connection_window->description_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    text_layer_set_text_color(connection_window->description_layer, GColorWhite);
    text_layer_set_background_color(connection_window->description_layer, GColorBlack);
    GSize max_size = graphics_text_layout_get_content_size(description, fonts_get_system_font(FONT_KEY_GOTHIC_24), description_layer_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter);
    max_size.w = WINDOW_FRAME.size.w;
    description_layer_frame.size.h = max_size.h;

    int content_height = description_layer_frame.origin.y+description_layer_frame.size.h+4;

    connection_window->background_layer = layer_create(GRect(0, 0, WINDOW_FRAME.size.w, content_height*PBL_IF_ROUND_ELSE(2, 1)));
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
    layer_destroy(connection_window->background_layer);
    text_layer_destroy(connection_window->title_layer);
    text_layer_destroy(connection_window->description_layer);
    scroll_layer_destroy(connection_window->root_layer);
}

void connection_window_send_test_message(){
    if(!connection_window){
        return;
    }

    iPodMessage *message = ipod_message_outbox_get();

    if(message->result != APP_MSG_OK){
        connection_window->error = ConnectionErrorOther;
        connection_window->error_code = message->result;
        return;
    }

    dict_write_int8(message->iter, MessageKeyConnectionTest, 1);

    AppMessageResult test_result = app_message_outbox_send();
    if(test_result != APP_MSG_OK){
        connection_window->error = ConnectionErrorOutboxDropped;
        connection_window->error_code = test_result;
    }
    else{
        connection_window->error = ConnectionErrorReconnecting;
    }
}

void connection_window_reload(ConnectionError error){
    connection_window_unload(connection_window->window);

    if(error == ConnectionErrorNoError){ //Automatically check for errors
        connection_window->pebblekit_connected = connection_service_peek_pebblekit_connection();
        connection_window->pebble_app_connected = connection_service_peek_pebble_app_connection();

        if(!connection_window->pebble_app_connected){
            connection_window->error = ConnectionErrorPebbleAppDisconnected;
        }
        else if(!connection_window->pebblekit_connected){
            connection_window->error = ConnectionErrorPebbleKitDisconnected;
        }
        else{
            connection_window_send_test_message();
        }
    }
    else{
        connection_window->error = error;
    }

    connection_window_load(connection_window->window);
}

void connection_window_push(ConnectionError error){
    if(connection_window){
        connection_window_reload(error);
        return;
    }
    connection_window = malloc(sizeof(ConnectionWindow));
    connection_window->error = error;

    connection_window->pebble_app_connected = connection_service_peek_pebble_app_connection();
    connection_window->pebblekit_connected = connection_service_peek_pebblekit_connection();

    if(!connection_window->pebble_app_connected){
        connection_window->error = ConnectionErrorPebbleAppDisconnected;
    }
    if(!connection_window->pebblekit_connected && connection_window->pebble_app_connected){
        connection_window->error = ConnectionErrorPebbleKitDisconnected;
    }

    connection_window->window = window_create();
    window_set_window_handlers(connection_window->window, (WindowHandlers){
        .load = connection_window_load,
        .unload = connection_window_unload
    });
    window_stack_push(connection_window->window, true);
}

void connection_window_inbox_dropped_handler(AppMessageResult reason, void *context){
    if(reason == APP_MSG_BUSY){
        connection_window_push(ConnectionErrorAppMessageBusy);
    }
    else{
        connection_window_push(ConnectionErrorOther);
    }
}

void connection_window_outbox_failed_handler(DictionaryIterator *iterator, AppMessageResult reason, void *context){
    connection_window_push(ConnectionErrorOutboxDropped);
}

void connection_window_pebble_app_connection_handler(bool connected){
    if(!connected){
        connection_window_push(ConnectionErrorPebbleAppDisconnected);
    }
    else if(connection_window){
        connection_window_push(ConnectionErrorNoError);
    }
}

void connection_window_pebblekit_connection_handler(bool connected){
    if(!connected){
        connection_window_push(ConnectionErrorPebbleKitDisconnected);
    }
    else if(connection_window){
        connection_window_push(ConnectionErrorNoError);
    }
}

void connection_window_got_test_message(){
    window_stack_pop(true);
    connection_window = NULL;
}

void connection_window_set_wakeup_cookie(WakeupCookie tasty_cookie){
    current_tasty_cookie = tasty_cookie;
}

void connection_window_attach(){
    ConnectionHandlers handlers = (ConnectionHandlers){
        .pebble_app_connection_handler = connection_window_pebble_app_connection_handler,
        .pebblekit_connection_handler = connection_window_pebblekit_connection_handler
    };
    connection_service_subscribe(handlers);

    app_message_register_inbox_dropped(connection_window_inbox_dropped_handler);
    app_message_register_outbox_failed(connection_window_outbox_failed_handler);
}

void connection_window_detach(){
    connection_service_unsubscribe();
    app_message_register_inbox_dropped(NULL);
    app_message_register_outbox_failed(NULL);
}