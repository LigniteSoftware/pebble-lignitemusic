#include <lignite_music.h>

iPodStateCallback ipod_state_callback;

GBitmap *album_art_bitmap[IMAGE_PARTS];
static uint8_t *album_art_data[IMAGE_PARTS];
static int now_playing_album_art_size[IMAGE_PARTS];

#ifndef PBL_PLATFORM_APLITE
GBitmap *library_icon_album_art_bitmap;
static uint8_t *library_icon_album_art_data;
static int header_icon_album_art_size;
#endif

MPMusicRepeatMode s_repeat_mode;
MPMusicShuffleMode s_shuffle_mode;
MPMusicPlaybackState s_playback_state;
NowPlayingType current_type;
uint32_t s_duration;
uint32_t s_current_time;
static char s_artist[100];
static char s_title[100];
bool already_requested_new_song = false;

void ipod_state_tick() {
    if(s_playback_state == MPMusicPlaybackStatePlaying) {
        if(s_current_time < s_duration) {
            ++s_current_time;
            already_requested_new_song = false;
        } else {
            if(already_requested_new_song){
                return;
            }

            now_playing_request(NowPlayingRequestTypeAllData);

            already_requested_new_song = true;
        }
    }
}

void ipod_state_set_callback(iPodStateCallback callback) {
    ipod_state_callback = callback;
}

uint16_t ipod_state_current_time() {
    return s_current_time;
}

uint16_t ipod_state_duration() {
    return s_duration;
}

MPMusicPlaybackState ipod_get_playback_state() {
    return s_playback_state;
}

MPMusicRepeatMode ipod_get_repeat_mode() {
    return s_repeat_mode;
}

MPMusicShuffleMode ipod_get_shuffle_mode() {
    return s_shuffle_mode;
}

char* ipod_get_artist() {
    return s_artist;
}

char* ipod_get_title() {
    return s_title;
}

bool first_open = true;
void now_playing_request(NowPlayingRequestType request_type) {
    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        return;
    }

    dict_write_int8(ipodMessage->iter, MessageKeyNowPlaying, request_type);
    dict_write_int8(ipodMessage->iter, MessageKeyWatchModel, watch_info_get_model());
    dict_write_int8(ipodMessage->iter, MessageKeyImageParts, IMAGE_PARTS);
    dict_write_int16(ipodMessage->iter, MessageKeyAppMessageSize, PHONE_MAX_BYTES);
    if(first_open){
        dict_write_int8(ipodMessage->iter, MessageKeyFirstOpen, true);
        first_open = false;
    }

    app_message_outbox_send();
}

bool first_call = true;
void call_callback(bool track_data) {
    if(first_call && !track_data){
        if(s_playback_state == MPMusicPlaybackStatePlaying){
            now_playing_show();
        }
        first_call = false;
    }
    if(!ipod_state_callback){
        return;
    }
    ipod_state_callback(track_data);
}

void destroy_all_album_art(){
    for(uint8_t i = 0; i < IMAGE_PARTS; i++){
        if(album_art_bitmap[i]){
            now_playing_set_album_art(i, NULL);
            gbitmap_destroy(album_art_bitmap[i]);
            album_art_bitmap[i] = NULL;
        }
    }
}

void create_bitmap(uint8_t image_part){
    if(album_art_bitmap[image_part]){
        return;
    }

    //NSLog("Creating bitmap with data %p, size %d and heap free %d.", album_art_data[image_part], now_playing_album_art_size[image_part], heap_bytes_free());
    album_art_bitmap[image_part] = gbitmap_create_from_png_data(album_art_data[image_part], now_playing_album_art_size[image_part]);

    GSize size = gbitmap_get_bounds(album_art_bitmap[image_part]).size;

    if(album_art_bitmap[image_part] == NULL || size.w == 0){
        display_no_album(image_part);
    }
    else{
        now_playing_set_album_art(image_part, album_art_bitmap[image_part]);
    }

    free(album_art_data[image_part]);
    album_art_data[image_part] = NULL;
}

void process_album_art_tuple(DictionaryIterator *albumArtDict){
    Tuple *albumArtImagePartTuple = dict_find(albumArtDict, MessageKeyImageParts);
    if(!albumArtImagePartTuple){
        return;
    }
    uint8_t image_part = albumArtImagePartTuple->value->uint8;

    Tuple *albumArtTuple = dict_find(albumArtDict, MessageKeyAlbumArt);
    if(albumArtTuple) {
        if(!album_art_data[image_part]){
            //NSError("Rejecting");
            return;
        }

        Tuple *albumArtIndexTuple = dict_find(albumArtDict, MessageKeyAlbumArtIndex);
        if(!albumArtIndexTuple){
            return;
        }
        size_t index = albumArtIndexTuple->value->uint16 * MAX_BYTES;
        //NSWarn("Got index %d for %p with length %d and total %d with heap free %d", albumArtIndexTuple->value->uint16, album_art_data[image_part], ((albumArtIndexTuple->value->uint16 * MAX_BYTES) + albumArtTuple->length), now_playing_album_art_size[image_part], heap_bytes_free());
        memcpy(&album_art_data[image_part][index], albumArtTuple->value->data, albumArtTuple->length);

        if(((albumArtIndexTuple->value->uint16 * MAX_BYTES) + albumArtTuple->length) == now_playing_album_art_size[image_part]){
            create_bitmap(image_part);
        }
    }
    else{
        Tuple* albumArtLengthTuple = dict_find(albumArtDict, MessageKeyAlbumArtLength);
        if(albumArtLengthTuple){
            if(image_part == 0){
                destroy_all_album_art();
            }

            uint16_t length = albumArtLengthTuple->value->uint16;

            if(album_art_data[image_part]){
                free(album_art_data[image_part]);
            }
            if(album_art_bitmap[image_part]){
                now_playing_set_album_art(image_part, NULL);
                gbitmap_destroy(album_art_bitmap[image_part]);
                album_art_bitmap[image_part] = NULL;
            }
            album_art_data[image_part] = malloc(length);
            now_playing_album_art_size[image_part] = length;

            if(now_playing_album_art_size[image_part] == 1){ //No album art
                now_playing_set_album_art(image_part, NULL);
            }
        }
    }
}

#ifndef PBL_PLATFORM_APLITE
void create_bitmap_for_header(){
    if(library_icon_album_art_bitmap){
        return;
    }
    library_icon_album_art_bitmap = gbitmap_create_from_png_data(library_icon_album_art_data, header_icon_album_art_size);

    GSize size = gbitmap_get_bounds(library_icon_album_art_bitmap).size;

    if(library_icon_album_art_bitmap == NULL || size.w == 0){
        library_menus_set_header_icon(NULL);
    }
    else{
        library_menus_set_header_icon(library_icon_album_art_bitmap);
    }

    free(library_icon_album_art_data);
    library_icon_album_art_data = NULL;
}

void process_library_menu_album_art_tuple(DictionaryIterator *albumArtDict){
    Tuple *albumArtTuple = dict_find(albumArtDict, MessageKeyHeaderIcon);
    if(albumArtTuple) {
        if(!library_icon_album_art_data){
            return;
        }

        Tuple *albumArtIndexTuple = dict_find(albumArtDict, MessageKeyHeaderIconIndex);
        if(!albumArtIndexTuple){
            return;
        }
        size_t index = albumArtIndexTuple->value->uint16 * MAX_BYTES;
        memcpy(&library_icon_album_art_data[index], albumArtTuple->value->data, albumArtTuple->length);

        if(((albumArtIndexTuple->value->uint16 * MAX_BYTES) + albumArtTuple->length) == header_icon_album_art_size){
            app_timer_register(125, create_bitmap_for_header, NULL);
        }
    }
    else{
        Tuple* albumArtLengthTuple = dict_find(albumArtDict, MessageKeyHeaderIconLength);
        //TODO check this shit
        if(albumArtLengthTuple){
            uint16_t length = albumArtLengthTuple->value->uint16;

            if(library_icon_album_art_data){
                free(library_icon_album_art_data);
            }
            if(library_icon_album_art_bitmap){
                library_menus_set_header_icon(NULL);
                gbitmap_destroy(library_icon_album_art_bitmap);
                library_icon_album_art_bitmap = NULL;
            }
            library_icon_album_art_data = malloc(length);
            if(!library_icon_album_art_data){
                NSError("Album art data FAILED to create with a size of %d bytes!", length);
            }
            header_icon_album_art_size = length;

            if(header_icon_album_art_size == 1){
                library_menus_set_header_icon(NULL);
            }
        }
    }
}
#endif

void process_tuple(Tuple *tuple, DictionaryIterator *iter){
    Settings current_settings = settings_get_settings();
    bool do_vibe = false;

    uint32_t key = tuple->key;
    if(key == MessageKeyCurrentState){
        s_playback_state = tuple->value->data[0];
        s_shuffle_mode = tuple->value->data[1];
        s_repeat_mode = tuple->value->data[2];
        s_duration = (tuple->value->data[3] << 8) | tuple->value->data[4];
        s_current_time = (tuple->value->data[5] << 8) | tuple->value->data[6];
        call_callback(false);
    }
    else if(key == MessageKeyNowPlayingResponseType){
        current_type = tuple->value->uint8;
    }
    else if(key == MessageKeyNowPlaying){
        Tuple *typeTuple = dict_find(iter, MessageKeyNowPlayingResponseType);
        if(!typeTuple){
            return;
        }
        current_type = typeTuple->value->uint8;

        char* target = NULL;
        switch(current_type) {
            case NowPlayingArtist:
                target = s_artist;
                break;
            case NowPlayingTitle:
                target = s_title;
                break;
            default:
                return;
        }
        if(strcmp(target, tuple->value->cstring) == 0){
            //NSWarn("Warning: Target string and cstring are equal. Not copying again.");
            return;
        }
        uint8_t len = strlen(tuple->value->cstring);
        if(len > 99){
            len = 99;
        }
        strncpy(target, tuple->value->cstring, (size_t)len);
        target[len] = '\0';
        call_callback(true);
    }
    else if(key == MessageKeyLibraryResponse){
        library_menus_inbox(iter);
    }
    else if(key == MessageKeySettingBatterySaver){
        current_settings.battery_saver = tuple->value->uint8;

        do_vibe = true;
    }
    else if(key == MessageKeySettingArtistLabel){
        current_settings.artist_label = tuple->value->uint8;

        do_vibe = true;
    }
    else if(key == MessageKeySettingPebbleStyleControls){
        current_settings.pebble_controls = tuple->value->uint8;

        do_vibe = true;
    }
    else if(key == MessageKeySettingShowTime){
        current_settings.show_time = tuple->value->uint8;

        do_vibe = true;
    }
    #ifndef PBL_PLATFORM_APLITE
    else if(key == MessageKeyHeaderIcon || key == MessageKeyHeaderIconLength){
        process_library_menu_album_art_tuple(iter);
    }
    #endif
    else if(key == MessageKeyAlbumArt || key == MessageKeyAlbumArtLength){
        process_album_art_tuple(iter);
    }
    else if(key == MessageKeyConnectionTest){
        connection_window_got_test_message();
    }

    settings_set_settings(current_settings);

    if(do_vibe){
        vibes_double_pulse();
    }
}

void ipod_received_handler(DictionaryIterator *iter, void *context){
    Tuple *t = dict_read_first(iter);
    if(t){
        process_tuple(t, iter);
    }
    while(t != NULL){
        t = dict_read_next(iter);
        if(t){
            process_tuple(t, iter);
        }
    }
}

void ipod_state_create() {
    app_message_register_inbox_received(ipod_received_handler);
    s_artist[0] = '\0';
    s_title[0] = '\0';
}