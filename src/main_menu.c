#include <pebble.h>
#include "main_menu.h"
#include "common.h"
#include "library_menus.h"
#include "now_playing.h"

void open_now_playing(int index, void* context);
void open_artist_list(int index, void* context);
void open_album_list(int index, void* context);
void open_playlist_list(int index, void* context);
void open_composer_list(int index, void* context);
void open_genre_list(int index, void* context);

const SimpleMenuItem main_menu_items[] = {
    {
        .title = "Now Playing",
        .callback = open_now_playing,
    },
    {
        .title = "Playlists",
        .callback = open_playlist_list,
    },
    {
        .title = "Artists",
        .callback = open_artist_list,
    },
    {
        .title = "Albums",
        .callback = open_album_list,
    },
    {
        .title = "Composers",
        .callback = open_composer_list,
    },
    {
        .title = "Genres",
        .callback = open_genre_list,
    }
};

const SimpleMenuSection section = {
    .title = NULL,
    .items = main_menu_items,
    .num_items = ARRAY_LENGTH(main_menu_items)
};

SimpleMenuLayer *main_menu_layer;

void main_menu_init(Window* window) {
    main_menu_layer = simple_menu_layer_create(GRect(0, 0, 144, 168), window, &section, 1, NULL);
    layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(main_menu_layer));
}

void open_now_playing(int index, void* context) {
    show_now_playing();
}
void open_artist_list(int index, void* context) {
    display_library_view(MPMediaGroupingAlbumArtist);
}
void open_album_list(int index, void* context) {
    display_library_view(MPMediaGroupingAlbum);
}
void open_playlist_list(int index, void* context) {
    display_library_view(MPMediaGroupingPlaylist);
}
void open_genre_list(int index, void* context) {
    display_library_view(MPMediaGroupingGenre);
}
void open_composer_list(int index, void* context) {
    display_library_view(MPMediaGroupingComposer);
}
