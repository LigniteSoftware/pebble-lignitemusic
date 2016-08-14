#include <lignite_music.h>

iPodStateCallback ipod_state_callback;

GBitmap *album_art_bitmap[IMAGE_PARTS], *library_icon_album_art_bitmap;
static uint8_t *album_art_data[IMAGE_PARTS], *library_icon_album_art_data;
static int now_playing_album_art_size[IMAGE_PARTS], header_icon_album_art_size = 0;

MPMusicRepeatMode s_repeat_mode;
MPMusicShuffleMode s_shuffle_mode;
MPMusicPlaybackState s_playback_state;
NowPlayingType current_type;
uint32_t s_duration;
uint32_t s_current_time;
static char s_album[100];
static char s_artist[100];
static char s_title[100];

void ipod_state_tick() {
    if(s_playback_state == MPMusicPlaybackStatePlaying) {
        if(s_current_time < s_duration) {
            ++s_current_time;
        } else {
            iPodMessage *ipodMessage = ipod_message_outbox_get();
            if(!ipodMessage->iter){
                free(ipodMessage);
                return;
            }
            dict_write_int8(ipodMessage->iter, MessageKeyNowPlaying, 2);
            app_message_outbox_send();
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

char* ipod_get_album() {
    return s_album;
}

char* ipod_get_artist() {
    return s_artist;
}

char* ipod_get_title() {
    return s_title;
}

//TODO: add support for these
void ipod_set_shuffle_mode(MPMusicShuffleMode shuffle) {}
void ipod_set_repeat_mode(MPMusicRepeatMode repeat) {}

void now_playing_request(bool force_reload) {
    // NSWarn("Rejecting request");
    // return;

    iPodMessage *ipodMessage = ipod_message_outbox_get();
    if(!ipodMessage->iter){
        NSError("Iter is null!");
        return;
    }

    //100 will indicate a force reload, any other value that's within int8_t and non-zero will
    //simply let iOS handle the logic (ie. not sending the same album art twice)
    uint8_t value = force_reload ? 100 : 200;
    dict_write_int8(ipodMessage->iter, MessageKeyNowPlaying, value);
    dict_write_int8(ipodMessage->iter, MessageKeyWatchModel, watch_info_get_model());
    dict_write_int8(ipodMessage->iter, MessageKeyImageParts, IMAGE_PARTS);
    dict_write_int16(ipodMessage->iter, MessageKeyAppMessageSize, APP_MESSAGE_SIZE-4);

    app_message_outbox_send();

    ipod_message_destroy(ipodMessage);
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
        //NSError("State callback doesn't exist! Gutlessly rejecting.");
        return;
    }
    ipod_state_callback(track_data);
}

void create_bitmap(uint8_t image_part){
    if(album_art_bitmap[image_part]){
        return;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG,"png_header for %p: %c%c%c", album_art_data[image_part], *((char*)(album_art_data[image_part]+1)), *((char*)(album_art_data[image_part]+2)), *((char*)(album_art_data[image_part]+3)));

    NSLog("Creating bitmap with data %p, size %d and heap free %d.", album_art_data[image_part], now_playing_album_art_size[image_part], heap_bytes_free());
    album_art_bitmap[image_part] = gbitmap_create_from_png_data(album_art_data[image_part], now_playing_album_art_size[image_part]);
    NSLog("Done. Got %p.", album_art_bitmap[image_part]);

    GSize size = gbitmap_get_bounds(album_art_bitmap[image_part]).size;

    if(album_art_bitmap[image_part] == NULL || size.w == 0){
        NSError("Nothing");
    }
    else{
        now_playing_set_album_art(image_part, album_art_bitmap[image_part]);
    }

    free(album_art_data[image_part]);
    album_art_data[image_part] = NULL;

    NSLog("Free %d", heap_bytes_free());
}

void process_album_art_tuple(DictionaryIterator *albumArtDict){
    Tuple *albumArtImagePartTuple = dict_find(albumArtDict, MessageKeyImageParts);
    if(!albumArtImagePartTuple){
        return;
    }
    uint8_t image_part = albumArtImagePartTuple->value->uint8;

    //NSLog("Got image part %d", image_part);

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
        //NSWarn("Got index %d with current length %d and total %d", albumArtIndexTuple->value->uint16, ((albumArtIndexTuple->value->uint16 * MAX_BYTES) + albumArtTuple->length), now_playing_album_art_size[image_part]);
        memcpy(&album_art_data[image_part][index], albumArtTuple->value->data, albumArtTuple->length);

        if(((albumArtIndexTuple->value->uint16 * MAX_BYTES) + albumArtTuple->length) == now_playing_album_art_size[image_part]){
            create_bitmap(image_part);
        }
    }
    else{
        Tuple* albumArtLengthTuple = dict_find(albumArtDict, MessageKeyAlbumArtLength);
        if(albumArtLengthTuple){
            uint16_t length = albumArtLengthTuple->value->uint16;

            NSLog("Got size for image %d: %d, heap free %d", image_part, length, heap_bytes_free());
            if(album_art_data[image_part]){
                NSLog("Freeing album art data for part %d.", image_part);
                free(album_art_data[image_part]);
            }
            if(album_art_bitmap[image_part]){
                now_playing_set_album_art(image_part, NULL);
                gbitmap_destroy(album_art_bitmap[image_part]);
                album_art_bitmap[image_part] = NULL;
            }
            album_art_data[image_part] = malloc(length);
            if(!album_art_data[image_part]){
                NSError("Album art data FAILED to create with a size of %d bytes!", length);
            }
            now_playing_album_art_size[image_part] = length;

            if(now_playing_album_art_size[image_part] == 1){ //No album art
                now_playing_set_album_art(image_part, NULL);
            }
            else{
                NSDebug("Ready for image input.");
            }
        }
    }
}

void create_bitmap_for_header(){
    if(library_icon_album_art_bitmap){
        return;
    }
    NSLog("Creating bitmap with data %p, size %d and heap free %d.", library_icon_album_art_data, header_icon_album_art_size, heap_bytes_free());
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

            NSLog("Got size for image %d, heap free %d", length, heap_bytes_free());
            if(library_icon_album_art_data){
                NSLog("Destroying album art data.");
                free(library_icon_album_art_data);
            }
            if(library_icon_album_art_bitmap){
                NSLog("Destroying album art bitmap.");
                library_menus_set_header_icon(NULL);
                gbitmap_destroy(library_icon_album_art_bitmap);
                library_icon_album_art_bitmap = NULL;
            }
            library_icon_album_art_data = malloc(length);
            if(!library_icon_album_art_data){
                NSError("Album art data FAILED to create with a size of %d bytes!", length);
            }
            header_icon_album_art_size = length;

            //You're probably not gonna have a PNG that's 1 byte lol, so if it is it's the phone telling you
            //that there's no fucking album art
            if(header_icon_album_art_size == 1){
                library_menus_set_header_icon(NULL);
            }
            else{
                NSLog("Ready for image input.");
            }
        }
    }
}

void process_tuple(Tuple *tuple, DictionaryIterator *iter){
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
            NSError("Type tuple doesn't exist!");
            return;
        }
        current_type = typeTuple->value->uint8;

        char* target = NULL;
        switch(current_type) {
            case NowPlayingAlbum:
                target = s_album;
                break;
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
        Settings current_settings = settings_get_settings();
        current_settings.battery_saver = tuple->value->uint8;
        settings_set_settings(current_settings);

        vibes_double_pulse();
    }
    else if(key == MessageKeySettingArtistLabel){
        Settings current_settings = settings_get_settings();
        current_settings.artist_label = tuple->value->uint8;
        settings_set_settings(current_settings);

        vibes_double_pulse();
    }
    else if(key == MessageKeySettingPebbleStyleControls){
        Settings current_settings = settings_get_settings();
        current_settings.pebble_controls = tuple->value->uint8;
        settings_set_settings(current_settings);

        vibes_double_pulse();
    }
    else if(key == MessageKeyHeaderIcon || key == MessageKeyHeaderIconLength || key == MessageKeyHeaderIconIndex){
        process_library_menu_album_art_tuple(iter);
    }
    else{
        process_album_art_tuple(iter);
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
    s_album[0] = '\0';
    s_title[0] = '\0';
}