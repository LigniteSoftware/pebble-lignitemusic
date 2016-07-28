#ifndef ipod_main_menu_h
#define ipod_main_menu_h

#define AMOUNT_OF_MAIN_MENU_ITEMS 6

#include <pebble.h>

typedef void (*MainMenuCallback)();

void main_menu_init(Window* window);

#endif
