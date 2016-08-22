#pragma once

typedef enum {
    ConnectionErrorPebbleAppDisconnected = 0,
    ConnectionErrorPebbleKitDisconnected,
    ConnectionErrorOutboxDropped,
    ConnectionErrorAppMessageBusy,
    ConnectionErrorOther
} ConnectionError;

typedef struct {
    Window *window;
    ScrollLayer *root_layer;
    Layer *background_layer;
    TextLayer *title_layer;
    TextLayer *description_layer;
    ConnectionError error;
    bool pebble_app_connected;
    bool pebblekit_connected;
    bool pushed;
} ConnectionWindow;

typedef void(*ConnectionWindowPushCallback)(bool opening);

void connection_window_attach();