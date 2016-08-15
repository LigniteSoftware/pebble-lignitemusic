#pragma once

typedef struct {
    Window *root_window;
    Layer *root_layer;
    GRect icon_frame;
    GRect text_frame;
    GBitmap *icon;
    char message[1][65];
    bool destroy_bitmap;
    bool is_pushed;
} MessageWindow;

typedef struct {
    MessageWindow *window;
} MessageWindowReference;

/**
 * Creates a new MessageWindow on the heap.
 * @return The newly created MessageWindow
 */
MessageWindow *message_window_create();

/**
 * Destroys a MessageWindow, freeing it from the heap.
 * @param window The MessageWindow to destroy.
 */
void message_window_destroy(MessageWindow *window);

/**
 * Shows a message window on top of another window.
 * @param message_window The MessageWindow to put on top of the Window.
 * @param root_window    The root window to add the MessageWindow to.
 * @param animated       Whether or not the window should animate in its duty.
 */
void message_window_push_on_window(MessageWindow *message_window, Window *root_window, bool animated);

/**
 * Pops a MessageWindow off of the current window that it is attached to.
 * @param message_window The MessageWindow to pop.
 * @param animated       Whether or not the window should animate in its duty.
 * @param auto_destroy   Whether or not to automatically destroy the MessageWindow once popped.
 * Any value greater than 0 will auto destroy it after X ms.
 */
void message_window_pop_off_window(MessageWindow *message_window, bool animated, int auto_destroy);

/**
 * Sets the text of the MessageWindow.
 * @param window The MessageWindow to set the text onto.
 * @param text   The text to set onto the MessageWindow. Automatically marks
 * the layer as dirty.
 */
void message_window_set_text(MessageWindow *window, char *text);

/**
 * Sets the icon of the MessageWindow. NULL to draw no icon.
 * @param window       The MessageWindow to set the icon onto.
 * @param icon         The icon to set onto the MessageWindow. Automatically marks
 * the layer as dirty.
 * @param auto_destroy Whether or not the MessageWindow should automatically
 * destroy the GBitmap when message_window_destroy is called on it.
 */
void message_window_set_icon(MessageWindow *window, GBitmap *icon, bool auto_destroy);