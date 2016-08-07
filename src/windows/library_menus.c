#include <lignite_music.h>

#define MENU_CACHE_COUNT 25
#define MENU_ENTRY_LENGTH 21
#define MENU_STACK_DEPTH 4 // Deepest: genres -> artists -> albums -> songs
#define MAX_MENU_ENTRIES 725

typedef struct {
    MenuLayer *layer;
    Window *window;
    char menu_entries[MENU_CACHE_COUNT][MENU_ENTRY_LENGTH];
    uint16_t total_entry_count;
    uint16_t current_entry_offset;
    uint16_t last_entry;
    uint16_t current_selection;
    MPMediaGrouping grouping;
} LibraryMenu;

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

bool build_parent_history(DictionaryIterator *iter) {
    if(menu_stack_count > 0) {
        // Pack up the parent stuff.
        uint8_t parents[MENU_STACK_DEPTH*3+1];
        parents[0] = menu_stack_count;
        for(uint8_t i = 0; i < menu_stack_count; ++i) {
            uint16_t current_selection = menu_stack[i]->current_selection;
            uint16_t second_current_selection = menu_stack[i]->current_selection;
            NSLog("Current selection is %d for %d", current_selection, i);
            parents[i*3+1] = menu_stack[i]->grouping;
            parents[i*3+3] = current_selection >> 8;
            parents[i*3+2] = current_selection & 0xFF;
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
    NSLog("Will play track at index %d", index);
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
    menu->total_entry_count = 0;
    menu->current_entry_offset = 0;
    menu->last_entry = 0;
    menu->current_selection = 0;
    memset(menu->menu_entries, 0, MENU_CACHE_COUNT * MENU_ENTRY_LENGTH);

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
    send_library_request(grouping, 0);
}

void library_menus_inbox(DictionaryIterator *received) {
    NSLog("Menu message");
    if(menu_stack_count == -1) {
        NSError("menu_stack_count is -1!");
        return;
    }

    Tuple* tuple = dict_find(received, MessageKeyLibraryResponse);
    LibraryMenu *menu = menu_stack[menu_stack_count];
    if(tuple) {
        MPMediaGrouping grouping = tuple->value->data[0];
        if(grouping != menu->grouping){
            NSError("grouping != menu->grouping!");
            return; // Not what we wanted.
        }
        uint32_t total_size = tuple->value->data[1];
        uint32_t offset = tuple->value->data[3];

        NSLog("Got total size %d, offset %d", (int)total_size, (int)offset);

        int8_t insert_pos = offset - menu->current_entry_offset;
        menu->total_entry_count = total_size;
        uint8_t skipping = 0;
        if(insert_pos < 0) {
            // Don't go further than 5 back.
            if(insert_pos < -5) {
                skipping = -5 - insert_pos;
            }
            memmove(&menu->menu_entries[5], &menu->menu_entries[0], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
            menu->last_entry += 5;
            menu->current_entry_offset = (menu->current_entry_offset < 5) ? 0 : menu->current_entry_offset - 5;
            if(menu->last_entry >= MENU_CACHE_COUNT){
                menu->last_entry = MENU_CACHE_COUNT - 1;
            }
            insert_pos = 0;
        }
        for(int i = insert_pos, j = 5; i < MENU_CACHE_COUNT && j < tuple->length; ++i) {
            uint8_t len = tuple->value->data[j++];
            if(skipping) {
                skipping--;
            }
            else {
                memset(menu->menu_entries[i], 0, MENU_ENTRY_LENGTH);
                memcpy(menu->menu_entries[i], &tuple->value->data[j], len < MENU_ENTRY_LENGTH - 1 ? len : MENU_ENTRY_LENGTH - 1);
                if(i > menu->last_entry) {
                    menu->last_entry = i;
                }
                if(i == MENU_CACHE_COUNT - 1) {
                    // Shift back if we're out of space, unless we're too far ahead already.
                    if(menu->current_selection - menu->current_entry_offset <= 10){
                        break;
                    }
                    memmove(&menu->menu_entries[0], &menu->menu_entries[5], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
                    menu->last_entry -= 5;
                    menu->current_entry_offset += 5;
                    i -= 5;
                }
            }
            j += len;
        }
        menu_layer_reload_data(menu->layer);
    }
}

// Window callbacks
void library_menus_window_unload(Window* window) {
    if(menu_stack_count > -1){
        menu_stack_count--;
    }

    LibraryMenu *library_menu = NULL;
    uint8_t i = 0;
    for(i = 0; i < MENU_STACK_DEPTH; i++){
        if(menu_stack[i]->window == window){
            library_menu = menu_stack[i];
            break;
        }
    }

    if(library_menu != NULL){
        menu_layer_destroy(library_menu->layer);
        window_destroy(library_menu->window);
        free(library_menu);
        menu_stack[i] = NULL;
    }
}

// Menu callbacks
uint16_t get_num_rows(MenuLayer* layer, uint16_t section_index, void* context) {
    uint16_t total_count = ((LibraryMenu*)context)->total_entry_count;
    return total_count < MAX_MENU_ENTRIES ? total_count : MAX_MENU_ENTRIES;
}

void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = cell_index->row - menu->current_entry_offset;
    if(pos >= MENU_CACHE_COUNT || pos < 0) {
        return;
    }

    menu_cell_basic_draw(ctx, cell_layer, menu->menu_entries[pos], NULL, NULL);
}

void selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = new_index.row - menu->current_entry_offset;
    menu->current_selection = new_index.row;
    NSLog("New selection is %d", menu->current_selection);
    bool down = new_index.row > old_index.row;

    if(down) {
        NSLog("Down");
        if(pos >= menu->last_entry - 4) {
            NSLog("%d (pos) >= %d (menu->last_entry - 4)", pos, (menu->last_entry - 4));
            if(menu->current_entry_offset + menu->last_entry >= menu->total_entry_count-1) {
                NSLog("%d (menu->current_entry_offset + menu->last_entry) >= %d (menu->total_entry_count)", (menu->current_entry_offset + menu->last_entry), menu->total_entry_count);
                return;
            }
            else{
                NSLog("%d (menu->current_entry_offset + menu->last_entry) AND %d (menu->total_entry_count)", (menu->current_entry_offset + menu->last_entry), menu->total_entry_count);
            }
            send_library_request(menu->grouping, menu->last_entry + menu->current_entry_offset);
        }
    } else {
        NSLog("Up");
        if(pos < 5 && menu->current_entry_offset > 0) {
            NSLog("pos < 5 && menu->current_entry_offset > 0");
            send_library_request(menu->grouping, menu->current_entry_offset > 5 ? menu->current_entry_offset - 5 : 0);
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
