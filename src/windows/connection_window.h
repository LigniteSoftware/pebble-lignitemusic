#pragma once

typedef enum {
    ConnectionErrorNoError = -1,
    ConnectionErrorReconnecting = 0,
    ConnectionErrorPebbleAppDisconnected,
    ConnectionErrorPebbleKitDisconnected,
    ConnectionErrorOutboxDropped,
    ConnectionErrorAppMessageBusy,
    ConnectionErrorOther
} ConnectionError;

typedef enum {
    WakeupCookieMainMenu = 0,
    WakeupCookieNowPlaying
} WakeupCookie;

typedef struct {
    Window *window;
    ScrollLayer *root_layer;
    Layer *background_layer;
    TextLayer *title_layer;
    TextLayer *description_layer;
    ConnectionError error;
    AppMessageResult error_code;
    bool pebble_app_connected;
    bool pebblekit_connected;
    bool pushed;
} ConnectionWindow;

typedef void(*ConnectionWindowPushCallback)(bool opening);

void connection_window_got_test_message();
void connection_window_attach();
void connection_window_set_wakeup_cookie(WakeupCookie tasty_cookie);