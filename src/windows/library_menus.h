#pragma once

#include <lignite_music.h>

//TODO: change this if there is less memory available.
#ifdef PBL_PLATFORM_APLITE
#define MENU_CACHE_COUNT 20
#else
#define MENU_CACHE_COUNT 30
#endif
#define MENU_ENTRY_LENGTH 21
#define MENU_STACK_DEPTH 4 // Deepest: genres -> artists -> albums -> songs
#define MAX_MENU_ENTRIES 725

typedef struct {
    char entries[MENU_CACHE_COUNT][MENU_ENTRY_LENGTH];
    uint16_t total_entry_count;
    uint16_t current_entry_offset;
    uint16_t last_entry;
} LibraryMenuEntryData;

typedef struct {
    Window *window;
    MenuLayer *layer;
    bool title_and_subtitle;
    bool has_autoselected;
    char title_text[1][MENU_ENTRY_LENGTH];
    char subtitle_text[1][MENU_ENTRY_LENGTH];
    GBitmap *header_icon;
    GBitmap *icon;
    GBitmap *icon_inverted;
    MPMediaGrouping grouping;
    uint16_t current_selection;
    LibraryMenuEntryData *titles;
    LibraryMenuEntryData *subtitles;
    MessageWindow *loading_window;
} LibraryMenu;

/**
 * Sets up the library menus for use.
 */
void library_menus_create();

/**
 * Gets rid of all library menus on the stack.
 */
void library_menus_pop_all();

/**
 * Sets the header icon for the LibraryMenu.
 * @param icon The icon to set.
 */
void library_menus_set_header_icon(GBitmap *icon);

/**
 * Displays a library view for a certain media grouping.
 * @param grouping The grouping to show data for.
 * @param title    If there is a title, having a string greater than 0 characters will
 * display this in a custom header
 * @param subtitle If there is a subtitle, having a string greater than 0 characters
 * will display this in a custom header
 */
void library_menus_display_view(MPMediaGrouping grouping, char *title, char *subtitle);

/**
 * The inbox for the library menu. It handles any incoming library details.
 * @param received The DictionaryIterator that should be processed
 * by the library menu.
 */
void library_menus_inbox(DictionaryIterator *received);