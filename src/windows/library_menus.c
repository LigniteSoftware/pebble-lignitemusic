#include <lignite_music.h>

LibraryMenu *menu_stack[MENU_STACK_DEPTH];
int8_t menu_stack_count;

bool send_library_request(MPMediaGrouping grouping, uint32_t offset);
bool play_track(uint16_t index);

uint16_t get_num_rows(MenuLayer* layer, uint16_t section_index, void* context);
void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
void selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
void select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

void library_menus_window_unload(Window* window);

void library_menus_create() {
    menu_stack_count = -1;
}

void library_menus_pop_all(){
    for(uint8_t i = 0; i < MENU_STACK_DEPTH; i++){
        if(menu_stack[i]){
            window_stack_remove(menu_stack[i]->window, false);
        }
    }
}

bool build_parent_history(DictionaryIterator *iter) {
    if(menu_stack_count > 0) {
        // Pack up the parent stuff.
        uint8_t parents[MENU_STACK_DEPTH*3+1];
        parents[0] = menu_stack_count;
        for(uint8_t i = 0; i < menu_stack_count; ++i) {
            parents[i*3+1] = menu_stack[i]->grouping;
            parents[i*3+3] = menu_stack[i]->current_selection >> 8;
            parents[i*3+2] = menu_stack[i]->current_selection & 0xFF;
        }
        if(dict_write_data(iter, MessageKeyRequestParent, parents, ARRAY_LENGTH(parents)) != DICT_OK) {
            return false;
        }
    }
    return true;
}

bool send_library_request(MPMediaGrouping grouping, uint32_t offset) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(ipodMessage->result != APP_MSG_OK){
        return false;
    }
    NSLog("Sending library request with grouping %d and offset %d.", grouping, (int)offset);
    dict_write_uint8(ipodMessage->iter, MessageKeyRequestLibrary, grouping);
    dict_write_uint32(ipodMessage->iter, MessageKeyRequestOffset, offset);
    build_parent_history(ipodMessage->iter);
    if(app_message_outbox_send() != APP_MSG_OK){
        return false;
    }
    return true;
}

bool play_track(uint16_t index) {
    LibraryMenu *menu = menu_stack[menu_stack_count];
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(ipodMessage->result != APP_MSG_OK){
        return false;
    }
    dict_write_uint8(ipodMessage->iter, MessageKeyRequestLibrary, menu->grouping);
    dict_write_uint16(ipodMessage->iter, MessageKeyPlayTrack, menu->current_selection);
    build_parent_history(ipodMessage->iter);
    if(app_message_outbox_send() != APP_MSG_OK){
        return false;
    }
    return true;
}

void clear_library_entry_data(LibraryMenuEntryData *data){
    data->total_entry_count = 0;
    data->current_entry_offset = 0;
    data->last_entry = 0;
    memset(data->entries, 0, MENU_CACHE_COUNT * MENU_ENTRY_LENGTH);
}

GBitmap *library_menus_gbitmap_from_media_grouping(MPMediaGrouping grouping){
    uint32_t resource_id = 0;
    switch(grouping){
        case MPMediaGroupingTitle:
            resource_id = RESOURCE_ID_ICON_TITLE;
            break;
        case MPMediaGroupingAlbum:
            resource_id = RESOURCE_ID_ICON_ALBUMS;
            break;
        case MPMediaGroupingAlbumArtist:
        case MPMediaGroupingArtist:
            resource_id = RESOURCE_ID_ICON_ARTISTS;
            break;
        case MPMediaGroupingComposer:
            resource_id = RESOURCE_ID_ICON_COMPOSERS;
            break;
        case MPMediaGroupingPlaylist:
            resource_id = RESOURCE_ID_ICON_PLAYLISTS;
            break;
        case MPMediaGroupingGenre:
            resource_id = RESOURCE_ID_ICON_GENRES;
            break;
        default:
            resource_id = RESOURCE_ID_ICON_NOW_PLAYING;
            break;
    }
    return gbitmap_create_with_resource(resource_id);
}

void library_menus_display_view(MPMediaGrouping grouping) {
    if(menu_stack_count >= MENU_STACK_DEPTH){
        NSError("Depth of menu stack too great at %d! Rejecting.", menu_stack_count);
        return;
    }

    menu_stack_count++;

    if(!menu_stack[menu_stack_count]){
        menu_stack[menu_stack_count] = malloc(sizeof(LibraryMenu));
    }
    else{
        NSWarn("Menu stack %p already exists (index %d).", menu_stack[menu_stack_count], menu_stack_count);
    }

    LibraryMenu* menu = menu_stack[menu_stack_count];
    menu->grouping = grouping;
    menu->current_selection = 0;

    menu->titles = malloc(sizeof(LibraryMenuEntryData));
    menu->subtitles = malloc(sizeof(LibraryMenuEntryData));
    clear_library_entry_data(menu->titles);
    clear_library_entry_data(menu->subtitles);

    menu->icon = library_menus_gbitmap_from_media_grouping(menu->grouping);
    menu->icon_inverted = library_menus_gbitmap_from_media_grouping(menu->grouping);
    replace_gbitmap_color(GColorWhite, GColorBlack, menu->icon_inverted, NULL);

    menu->window = window_create();

    menu->layer = menu_layer_create(GRect(0, 0, 144, 168));

    menu_layer_set_click_config_onto_window(menu->layer, menu->window);
    menu_layer_set_callbacks(menu->layer, menu, (MenuLayerCallbacks){
        .get_num_rows = get_num_rows,
        .draw_row = draw_row,
        .selection_changed = selection_changed,
        .select_click = select_click,
    });
    menu_layer_set_normal_colors(menu->layer, GColorBlack, GColorWhite);
    menu_layer_set_highlight_colors(menu->layer, GColorWhite, GColorBlack);
    layer_add_child(window_get_root_layer(menu->window), menu_layer_get_layer(menu->layer));

    window_stack_push(menu->window, true);

    window_set_window_handlers(menu->window, (WindowHandlers) {
        .unload = library_menus_window_unload,
    });

    menu->loading_window = message_window_create();
    message_window_set_text(menu->loading_window, "Loading...");
    message_window_set_icon(menu->loading_window, menu->icon, false);
    message_window_push_on_window(menu->loading_window, menu->window, false);

    send_library_request(grouping, 0);
}

void library_menus_window_unload(Window* window) {
    if(menu_stack_count > -1){
        menu_stack_count--;
    }

    LibraryMenu *library_menu = NULL;
    uint8_t i = 0;
    for(i = 0; i < MENU_STACK_DEPTH; i++){
        if(menu_stack[i]){
            if(menu_stack[i]->window == window){
                library_menu = menu_stack[i];
                break;
            }
        }
    }

    if(library_menu != NULL){
        menu_layer_destroy(library_menu->layer);
        window_destroy(library_menu->window);

        if(library_menu->loading_window){
            message_window_destroy(library_menu->loading_window);
            library_menu->loading_window = NULL;
        }

        gbitmap_destroy(library_menu->icon);
        gbitmap_destroy(library_menu->icon_inverted);

        free(library_menu->titles);
        free(library_menu->subtitles);

        free(library_menu);
        menu_stack[i] = NULL;
    }
}

void library_menus_clean_up(void *void_menu){
    if(!void_menu){
        NSWarn("Caught you little bitch");
        return;
    }
    LibraryMenu *menu = (LibraryMenu*)void_menu;
    menu->loading_window = NULL;
}

void library_menus_inbox(DictionaryIterator *received) {
    if(menu_stack_count == -1) {
        NSError("menu_stack_count is -1!");
        return;
    }

    Tuple* tuple = dict_find(received, MessageKeyLibraryResponse);
    LibraryMenu *menu = menu_stack[menu_stack_count];
    if(tuple) {
        MPMediaGrouping grouping = tuple->value->data[0];
        bool is_subtitles = false;
        if(grouping != menu->grouping){
            is_subtitles = (menu->grouping == MPMediaGroupingAlbum && grouping == MPMediaGroupingAlbumArtist) //Artist name for albums
                                || (menu->grouping == MPMediaGroupingTitle && grouping == MPMediaGroupingPodcastTitle) //Artist name and track duration for tracks
                                || (menu->grouping == MPMediaGroupingPlaylist && grouping == MPMediaGroupingPodcastTitle); //Playlist song count

            if(!is_subtitles){
                NSError("grouping != menu->grouping!");
                return; // Not what we wanted.
            }
        }
        uint32_t total_size = tuple->value->data[1];
        uint32_t offset = tuple->value->data[3];

        NSLog("Got total size %d, offset %d, is_subtitles %d", (int)total_size, (int)offset, is_subtitles);

        LibraryMenuEntryData *entry_data = is_subtitles ? menu->subtitles : menu->titles;

        int8_t insert_pos = offset - entry_data->current_entry_offset;
        entry_data->total_entry_count = total_size;
        uint8_t skipping = 0;
        if(insert_pos < 0) {
            // Don't go further than 5 back.
            if(insert_pos < -5) {
                skipping = -5 - insert_pos;
            }
            memmove(&entry_data->entries[5], &entry_data->entries[0], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
            entry_data->last_entry += 5;
            entry_data->current_entry_offset = (entry_data->current_entry_offset < 5) ? 0 : entry_data->current_entry_offset - 5;
            if(entry_data->last_entry >= MENU_CACHE_COUNT){
                entry_data->last_entry = MENU_CACHE_COUNT - 1;
            }
            insert_pos = 0;
        }
        for(int i = insert_pos, j = 5; i < MENU_CACHE_COUNT && j < tuple->length; ++i) {
            uint8_t len = tuple->value->data[j++];
            if(skipping) {
                skipping--;
            }
            else {
                memset(entry_data->entries[i], 0, MENU_ENTRY_LENGTH);
                memcpy(entry_data->entries[i], &tuple->value->data[j], len < MENU_ENTRY_LENGTH - 1 ? len : MENU_ENTRY_LENGTH - 1);
                if(i > entry_data->last_entry) {
                    entry_data->last_entry = i;
                }
                if(i == MENU_CACHE_COUNT - 1) {
                    // Shift back if we're out of space, unless we're too far ahead already.
                    if(menu->current_selection - entry_data->current_entry_offset <= 10){
                        break;
                    }
                    memmove(&entry_data->entries[0], &entry_data->entries[5], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
                    entry_data->last_entry -= 5;
                    entry_data->current_entry_offset += 5;
                    i -= 5;
                }
            }
            j += len;
        }

        menu_layer_reload_data(menu->layer);
        message_window_pop_off_window(menu->loading_window, true, false);
        app_timer_register(500, library_menus_clean_up, menu);
    }
}

uint16_t get_num_rows(MenuLayer* layer, uint16_t section_index, void* context) {
    uint16_t total_count = ((LibraryMenu*)context)->titles->total_entry_count;
    if(total_count == 0){
        total_count = 1;
    }
    return total_count < MAX_MENU_ENTRIES ? total_count : MAX_MENU_ENTRIES;
}

void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = cell_index->row - menu->titles->current_entry_offset;
    if(pos >= MENU_CACHE_COUNT || pos < 0) {
        return;
    }

    GBitmap *icon_to_draw = menu_cell_layer_is_highlighted(cell_layer) ? menu->icon_inverted : menu->icon;

    //NSLog("Drawing for position %d", pos);
    if(menu->titles->total_entry_count == 0){
        menu_cell_basic_draw(ctx, cell_layer, "Loading...", NULL, icon_to_draw);
    }
    else{
        if(strcmp(menu->subtitles->entries[pos], "") == 0){
            menu_cell_basic_draw(ctx, cell_layer, menu->titles->entries[pos], NULL, icon_to_draw);
        }
        else{
            menu_cell_basic_draw(ctx, cell_layer, menu->titles->entries[pos], menu->subtitles->entries[pos], icon_to_draw);
        }
    }
}

void selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = new_index.row - menu->titles->current_entry_offset;
    menu->current_selection = new_index.row;
    //NSLog("New selection is %d", menu->current_selection);
    bool down = new_index.row > old_index.row;

    if(down) {
        if(pos >= menu->titles->last_entry - 4) {
            if(menu->titles->current_entry_offset + menu->titles->last_entry >= menu->titles->total_entry_count-1) {
                return;
            }
            send_library_request(menu->grouping, menu->titles->last_entry + menu->titles->current_entry_offset);
        }
    } else {
        if(pos < 5 && menu->titles->current_entry_offset > 0) {
            send_library_request(menu->grouping, menu->titles->current_entry_offset > 5 ? menu->titles->current_entry_offset - 5 : 0);
        }
    }
}

void select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    if(menu->grouping == MPMediaGroupingTitle) {
        play_track(cell_index->row);
        now_playing_show();
    } else {
        if(menu_stack_count + 1 >= MENU_STACK_DEPTH) return;
        if(menu->grouping != MPMediaGroupingAlbum && menu->grouping != MPMediaGroupingPlaylist) {
            if(menu->grouping == MPMediaGroupingGenre) {
                library_menus_display_view(MPMediaGroupingArtist);
            } else {
                library_menus_display_view(MPMediaGroupingAlbum);
            }
        } else {
            library_menus_display_view(MPMediaGroupingTitle);
        }
    }
}
