#pragma once

#define MENU_CACHE_COUNT 50
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
    MPMediaGrouping grouping;
    uint16_t current_selection;
    LibraryMenuEntryData *titles;
    LibraryMenuEntryData *subtitles;
} LibraryMenu;

/**
 * Sets up the library menus for use.
 */
void library_menus_create();

/**
 * Displays a library view for a certain media grouping.
 * @param grouping The grouping to show data for.
 */
void library_menus_display_view(MPMediaGrouping grouping);

/**
 * The inbox for the library menu. It handles any incoming library details.
 * @param received The DictionaryIterator that should be processed
 * by the library menu.
 */
void library_menus_inbox(DictionaryIterator *received);