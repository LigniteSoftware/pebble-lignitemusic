#pragma once

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