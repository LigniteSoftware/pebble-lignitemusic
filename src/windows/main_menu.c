#include <lignite_music.h>

void open_now_playing(int index, void* context);
void open_artist_list(int index, void* context);
void open_album_list(int index, void* context);
void open_playlist_list(int index, void* context);
void open_composer_list(int index, void* context);
void open_genre_list(int index, void* context);

uint32_t resource_ids[AMOUNT_OF_MAIN_MENU_ITEMS] = {
    RESOURCE_ID_ICON_NOW_PLAYING, RESOURCE_ID_ICON_PLAYLISTS,
    RESOURCE_ID_ICON_ARTISTS, RESOURCE_ID_ICON_ALBUMS,
    RESOURCE_ID_ICON_COMPOSERS, RESOURCE_ID_ICON_GENRES
};

SimpleMenuItem main_menu_items[AMOUNT_OF_MAIN_MENU_ITEMS];

MenuLayer *main_menu_layer = NULL;
GBitmap *main_menu_icons[AMOUNT_OF_MAIN_MENU_ITEMS];
GBitmap *main_menu_icons_inverted[AMOUNT_OF_MAIN_MENU_ITEMS];

Window *main_menu_window;

uint16_t main_menu_get_num_sections(MenuLayer *menu_layer, void *data) {
    return 1;
}

uint16_t menu_get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return AMOUNT_OF_MAIN_MENU_ITEMS;
}

int16_t menu_get_header_height_callback(MenuLayer *menu_layer, uint16_t section_index, void *data) {
    return 0;
}

#ifdef PBL_ROUND
int16_t menu_get_cell_height(MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context){
    return MENU_CELL_ROUND_FOCUSED_TALL_CELL_HEIGHT;
}
#endif

MenuIndex highlight_index;

void update_highlight_colours(){
    if(highlight_index.row != 0){
        menu_layer_set_highlight_colors(main_menu_layer, GColorWhite, GColorBlack);
    }
    else{
        menu_layer_set_highlight_colors(main_menu_layer, GColorRed, GColorWhite);
    }
    layer_mark_dirty(menu_layer_get_layer(main_menu_layer));
}

void menu_selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context){
    highlight_index = new_index;
    app_timer_register(PBL_IF_ROUND_ELSE(50, 125), update_highlight_colours, NULL);
}

void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);

    main_menu_items[cell_index->row].icon = main_menu_icons[cell_index->row];

    if(menu_cell_layer_is_highlighted(cell_layer)){
        if(cell_index->row != 0){
            main_menu_items[cell_index->row].icon = main_menu_icons_inverted[cell_index->row];
        }
    }

    menu_cell_basic_draw(ctx, cell_layer, main_menu_items[cell_index->row].title, NULL, main_menu_items[cell_index->row].icon);
}

void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
    main_menu_items[cell_index->row].callback(cell_index->row, data);
}

void main_menu_send_now_playing_request(){
    now_playing_request(true);
}

void main_menu_destroy_warning(void *warning){
    message_window_pop_off_window((MessageWindow*)warning, true, false);
}

void main_menu_create(Window* window) {
    if(!main_menu_window && window){
        main_menu_window = window;
    }
    if(main_menu_window && !window){
        window = main_menu_window;
    }
    if(!main_menu_window && !window){
        NSError("Windows error: both windows are NULL. Rejecting, gutlessly.");
        return;
    }

    static char *titles[AMOUNT_OF_MAIN_MENU_ITEMS] = {
        "Now Playing", "Playlists", "Artists", "Albums", "Composers", "Genres"
    };

    static MainMenuCallback callbacks[AMOUNT_OF_MAIN_MENU_ITEMS] = {
        open_now_playing, open_playlist_list, open_artist_list, open_album_list,
        open_composer_list, open_genre_list
    };

    for(int i = 0; i < AMOUNT_OF_MAIN_MENU_ITEMS; i++){
        main_menu_items[i].title = titles[i];

        #ifndef PBL_PLATFORM_APLITE
        main_menu_icons[i] = gbitmap_create_with_resource(resource_ids[i]);
        main_menu_icons_inverted[i] = gbitmap_create_with_resource(resource_ids[i]);
        replace_gbitmap_color(GColorWhite, GColorBlack, main_menu_icons_inverted[i], NULL);
        #endif

        main_menu_items[i].callback = callbacks[i];
    }

    main_menu_layer = menu_layer_create(WINDOW_FRAME);
    menu_layer_set_normal_colors(main_menu_layer, GColorBlack, GColorWhite);
    menu_layer_set_highlight_colors(main_menu_layer, GColorRed, GColorWhite);
    menu_layer_set_callbacks(main_menu_layer, NULL, (MenuLayerCallbacks){
        .get_num_sections = main_menu_get_num_sections,
        .get_num_rows = menu_get_num_rows_callback,
        .get_header_height = menu_get_header_height_callback,
        .draw_row = menu_draw_row_callback,
        .select_click = menu_select_callback,
        #ifdef PBL_ROUND
        .get_cell_height = menu_get_cell_height,
        #endif
        .selection_changed = menu_selection_changed
    });
    menu_layer_set_click_config_onto_window(main_menu_layer, window);
    layer_add_child(window_get_root_layer(window), menu_layer_get_layer(main_menu_layer));

    app_timer_register(250, main_menu_send_now_playing_request, NULL);

    if(!bluetooth_connection_service_peek()){ //Phone not connected
        MessageWindow *not_connected_window = message_window_create();
        message_window_set_text(not_connected_window, "Your phone isn't connected! Lignite Music needs a phone...");
        message_window_push_on_window(not_connected_window, window, true);
        app_timer_register(10000, main_menu_destroy_warning, not_connected_window);

        vibes_long_pulse();
    }

}

void main_menu_destroy(){
    NSDebug("Before destroy %d", heap_bytes_free());
    #ifndef PBL_PLATFORM_APLITE
    for(int i = 0; i < AMOUNT_OF_MAIN_MENU_ITEMS; i++){
        gbitmap_destroy(main_menu_icons[i]);
        gbitmap_destroy(main_menu_icons_inverted[i]);
    }
    #endif

    menu_layer_destroy(main_menu_layer);
    NSDebug("After destroy %d", heap_bytes_free());
}

void open_now_playing(int index, void *context) {
    now_playing_show();
}
void open_artist_list(int index, void *context) {
    library_menus_display_view(MPMediaGroupingAlbumArtist, "", "");
}
void open_album_list(int index, void *context) {
    library_menus_display_view(MPMediaGroupingAlbum, "", "");
}
void open_playlist_list(int index, void *context) {
    library_menus_display_view(MPMediaGroupingPlaylist, "", "");
}
void open_genre_list(int index, void *context) {
    library_menus_display_view(MPMediaGroupingGenre, "", "");
}
void open_composer_list(int index, void *context) {
    library_menus_display_view(MPMediaGroupingComposer, "", "");
}
