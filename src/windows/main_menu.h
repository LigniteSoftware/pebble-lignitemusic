#pragma once

//The amount of items in this main menu.
#define AMOUNT_OF_MAIN_MENU_ITEMS 6

#include <lignite_music.h>

/**
 * The MainMenuCallback is simply a callback definition for main menu items
 * being clicked.
 */
typedef void (*MainMenuCallback)();

/**
 * Creates and sets up the main menu and contents.
 * @param window The window to attach the menu onto.
 */
void main_menu_create(Window* window);